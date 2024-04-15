#include "LOGIC.h"
#include "QUEUES.h"
#include <cstdio>
#include <cstdlib>
#include "MbedJSONValue.h"




void logic_thread(){

    PomiarSensor *sensor;  
    PomiarSensor *mail; 
    MbedJSONValue JSON;
    while (true) {
    
            sensor = pomiar_mail_box.try_get();
    
        if(sensor!=nullptr){
                

                // pomiar_mutex.lock();

                // printf("Temp: %.2f degC\n" , sensor->temp); 
                // printf("Hum: %.2f %%\n" , sensor->hum); 
                // printf("Press: %.2f hPa\n" , sensor->press);
                // printf("Time: %s\n", sensor->time);
                // printf("Numer pomiaru: %d\n" , sensor->n);
                // printf("FLAG: %d\n\n\n" , event_netinit_done.get());
                // ThisThread::sleep_for(10s);
                // // paczka = to_json(sensor); // umieszczenie stringa w paczka na podstawie funcji serializującej structa sensor
                
                
                // ===========================================1=================================
                mail = comm_mail_box.try_alloc(); //przypisanie pamieci do wysylki string (teraz struct) do watku COMM
                *mail = *sensor;
                // int pres = static_cast<int>(std::ceil(sensor->press));
                // // printf("pres: %d\n" , pres);

                // JSON["Device"] = "eval_status";
                // JSON["Temp"] = sensor->temp;
                // JSON["Hum"] = sensor->hum;
                // JSON["Press"] = pres;
                // JSON["NumerPomiaru"] = sensor->n;
                // JSON["Time"] = sensor->time;
                // string paczka_string = JSON.serialize();
                // paczka->mail = paczka_string.c_str(); //konwersja string na const char *
                // printf("\nPRINT_MEASUREMENT STRING: \n %s", paczka->mail);

                // MbedJSONValue JSON;
                // JSON["Edge_Device"] [0] = "eval_status";
                // JSON["Temp"] [0] = sensor->temp;
                // JSON["Hum"] [0] = sensor->hum;
                // JSON["Press"] [0] = pres;
                // JSON["Numerpomiaru"] [0] = sensor->n;
                // JSON["Time"] [0] = sensor->time;
                // string paczka_string = JSON.serialize();
                // paczka->mail = paczka_string.c_str(); //konwersja string na const char *
               
                // ===========================================2=================================

                
                if (!comm_mail_box.full()) {
                    
                osStatus q = comm_mail_box.put(mail);
                // printf(" PUT OK from READ THREAD to COMM MAIL:%d\n",q);  //wysłanie stinga do kolejki comm_queue do wątku COMM
                }

                // =====================wewnetrzne sprawdzenie w threadzie======================
                // paczka = comm_mail_box.try_get();
                // printf("%s\n", paczka->mail); //drukowanie odczytu przeslanego z READ_SENSORS
                
                // osStatus y = comm_mail_box.free(paczka);
                // printf("\n Status on memory clear MAILBOX: %d\n",y);
                // ==============================================================================


                osStatus w = pomiar_mail_box.free(sensor); //uwolnienie pamięci kolejki pomiar_mpool
                // printf("\n Status on memory clear: %d\n",w);
                if (comm_mail_box.full()) {
                 printf("\n COMM MAIL BOX IS FULL!");
                 return;
                }
                if (sensor == NULL) {
                    printf("Memory alloc fail!");
                }                
               
        }
        
    }

}