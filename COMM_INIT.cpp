#include "COMM_INIT.h"
#include "QUEUES.h"
#include <cstdio>

// Globalny symbol, do którego odwołuje się port Azure SDK dla Mbed OS, poprzez "extern"
NetworkInterface *_defaultSystemNetwork;

void net_init_thread()
{     
    // wątek czeka na flagę NET_INIT_START
    while(true){
       
        if (event.wait_any(NET_INIT_START, osWaitForever, false))
        {  
            // podniesienie flagi przed pętlą
            LogInfo("Connecting to the network");

            _defaultSystemNetwork = NetworkInterface::get_default_instance();
            _defaultSystemNetwork->set_default_parameters();

            if (_defaultSystemNetwork == nullptr) {
                LogError("No network interface found");
            }
            int ret = _defaultSystemNetwork->connect();
            if (ret != 0) {
                LogError("Connection error: %d", ret);
            }
            LogInfo("Connection success, MAC: %s", _defaultSystemNetwork->get_mac_address());
            LogInfo("Getting time from the NTP server");

            NTPClient ntp(_defaultSystemNetwork);
            ntp.set_server("time.google.com", 123);
            time_t time = ntp.get_timestamp();
            if (time < 0) {
                LogError("Failed to get the current time, error: %c", time);
            }
            
            set_time(time);

            LogInfo("Time: %s", ctime(&time));
        
            //wyczyszczenie flagi w petli
            event.clear(NET_INIT_START);
            event.set(NETINIT_COMPLETE);
            
        }
            
    }      
}