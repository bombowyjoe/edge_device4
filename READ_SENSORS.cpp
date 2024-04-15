#include "READ_SENSORS.h"
#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"
#include "QUEUES.h"

#define CZESTOTLIWOSC_POMIARU 60s


void read_thread(){

   
    // przypisanie PINow 
    DevI2C devI2c(PB_11,PB_10);
    LPS22HBSensor press_temp(&devI2c,0xBB); 
    HTS221Sensor hum_temp(&devI2c,0xBF);

    // inicjalicacja urzadzen pomiarowych
    
    hum_temp.init(NULL);
    hum_temp.enable();

    press_temp.init(NULL);
    press_temp.enable();
    
    
    time_t time_local;

    uint32_t inc_n = 0;

    PomiarSensor *pomiar;

    
    while (true){

    if(!pomiar_mail_box.full() && event.wait_all(AZURE_INIT_COMPLETE, osWaitForever, false)){
        

        pomiar = pomiar_mail_box.try_alloc();
        
        // numer pomiaru
        inc_n++;
        // inkrementacja numeru pomiaru
        pomiar->n = inc_n;
        // ODCZYT TEMPEATURY I WILGOTNOŚCI
        hum_temp.get_temperature(&pomiar->temp);
        hum_temp.get_humidity(&pomiar->hum);

        // ODCZYT TEMPEATURY I CIŚNIENIA
        press_temp.get_pressure(&pomiar->press);

        // zapis czasu odczytu
        time_local = time(NULL);
        pomiar->time = ctime(&time_local); //zamiana czasu na string
        
        pomiar_mail_box.put(pomiar);

        ThisThread::sleep_for(CZESTOTLIWOSC_POMIARU);

        }
    else {
    printf("POMIAR MAILBOX IS FULL!");
    ThisThread::sleep_for(1s);
    return;
    }
  }
}