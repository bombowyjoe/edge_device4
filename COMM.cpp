#include "COMM.h"
#include "Json.h"
#include "azure_cloud_credentials.h"
#include "jsmn.h"
#include <cstdlib>
#include <cstring>
#include "MbedJSONValue.h"

#define LICZBA_POMIAROW_W_PACZCE 5

bool trace_on = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;
tickcounter_ms_t interval = 100;
IOTHUB_CLIENT_RESULT res;
bool message_received = false;
int n=0;

void on_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) {
        LogInfo("Connected to IoT Hub");
    } else {
        LogError("Connection failed, reason: %s", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
    }
}

IOTHUBMESSAGE_DISPOSITION_RESULT on_message_received(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    LogInfo("Message received from IoT Hub");

    const unsigned char *data_ptr;
    size_t len;
    if (IoTHubMessage_GetByteArray(message, &data_ptr, &len) != IOTHUB_MESSAGE_OK) {
        LogError("Failed to extract message data, please try again on IoT Hub");
        return IOTHUBMESSAGE_ABANDONED;
    }

    message_received = true;
    LogInfo("Message body: %.*s", len, data_ptr);
    return IOTHUBMESSAGE_ACCEPTED;
}

void on_message_sent(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        LogInfo("Message sent successfully");
    } else {
        LogInfo("Failed to send message, error: %s",
            MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    }
}

string Serialize(PomiarSensor *paczka, int pres) {
MbedJSONValue JSON;
  JSON["Device"] = "eval_status";
  JSON["Temp"] = paczka->temp;
  JSON["Hum"] = paczka->hum;
  JSON["Press"] = pres;
  JSON["NumerPomiaru"] = paczka->n;
  JSON["Time"] = paczka->time;
  string paczka_string = JSON.serialize();
  return paczka_string;
}

void comm_thread() {

  event.wait_all(NETINIT_COMPLETE, osWaitForever, false);
  
  // Zmiana nastawy czasu wygasania certyfikatu SAS
  size_t sas_lifetime = 86400;
  string full_paczka_string;

  LogInfo("Initializing IoT Hub client");

  IoTHub_Init();

  IOTHUB_DEVICE_CLIENT_HANDLE client_handle =
      IoTHubDeviceClient_CreateFromConnectionString(
          azure_cloud::credentials::iothub_connection_string, MQTT_Protocol);

  if (client_handle == nullptr) {
    LogError("string %s", azure_cloud::credentials::iothub_connection_string);
    LogError("Failed to create IoT Hub client handle");
    goto end;
  }

  // Enable SDK tracing
  res =
      IoTHubDeviceClient_SetOption(client_handle, OPTION_LOG_TRACE, &trace_on);
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to enable IoT Hub client tracing, error: %d", res);
    goto end;
  }

  // Enable static CA Certificates defined in the SDK
  res = IoTHubDeviceClient_SetOption(client_handle, OPTION_TRUSTED_CERT,
                                     certificates);
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to set trusted certificates, error: %d", res);
    goto end;
  }

  // Process communication every 100ms
  res = IoTHubDeviceClient_SetOption(client_handle,
                                     OPTION_DO_WORK_FREQUENCY_IN_MS, &interval);
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to set communication process frequency, error: %d", res);
    goto end;
  }

  // set incoming message callback
  res = IoTHubDeviceClient_SetMessageCallback(client_handle,
                                              on_message_received, nullptr);
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to set message callback, error: %d", res);
    goto end;
  }

  // Set connection/disconnection callback
  res = IoTHubDeviceClient_SetConnectionStatusCallback(
      client_handle, on_connection_status, nullptr);
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to set connection status callback, error: %d", res);
    goto end;
  }

  // Set retry policy - added not to disconnect because of SAS lifetime
  res = IoTHubDeviceClient_SetRetryPolicy(
      client_handle, IOTHUB_CLIENT_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER, 1);
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to set connection status callback, error: %d", res);
    goto end;
  }

  

  res = IoTHubDeviceClient_SetOption(
      client_handle, OPTION_SAS_TOKEN_LIFETIME, &sas_lifetime );
  if (res != IOTHUB_CLIENT_OK) {
    LogError("Failed to set SAS override, error: %d", res);
    goto end;    
  }


  // komunikacja:

  event.set(AZURE_INIT_COMPLETE);

  while (true) {

    if (event.wait_all(AZURE_INIT_COMPLETE ,
    osWaitForever, false)) {

    bool trace_on = MBED_CONF_APP_IOTHUB_CLIENT_TRACE;
    tickcounter_ms_t interval = 100;
    IOTHUB_CLIENT_RESULT res;
    PomiarSensor *paczka;
    
    paczka = comm_mail_box.try_get();
        
    IOTHUB_MESSAGE_HANDLE message_handle;
    
    if (paczka != nullptr) {
    int pres = static_cast<int>(std::ceil(paczka->press));
    
    string paczka_string = Serialize(paczka,pres);

    full_paczka_string.append(paczka_string);

    osStatus y = comm_mail_box.free(paczka);

    paczka_string.clear();

    n++;
    }
    
    if ( n == LICZBA_POMIAROW_W_PACZCE ){
      const char * mail = full_paczka_string.c_str(); // konwersja string na const char *
    
      LogInfo("\nSending from COMM paczka->mail: \n %s", mail);

      message_handle = IoTHubMessage_CreateFromString(mail);

      res = IoTHubDeviceClient_SendEventAsync(client_handle, message_handle,
                                              on_message_sent, nullptr);

      IoTHubMessage_Destroy(
          message_handle); // wiadomość skopiowana do SDK, można uwolnić pamięć
     
    full_paczka_string.clear();
    n=0;
    
    if (message_handle == nullptr) {
      LogError("Failed to create message");
    goto end;
    }

    if (res != IOTHUB_CLIENT_OK) {
      LogError("Failed to send message event, error: %d", res);
    goto end;
    }
    
    }
    
  }
  end:
    int a = 1;
//   IoTHub_Deinit();
  }
}