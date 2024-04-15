#include "mbed.h"
#include "rtos.h"

#include "READ_SENSORS.h"
#include "LOGIC.h"
#include "QUEUES.h"
#include "COMM_INIT.h"
#include "COMM.h"
#include "config.h"

#include <cstdio>

#define BLINKING_RATE   500ms

// przypisanie I/O
InterruptIn button_push(PC_13);
PwmOut led(LED1);
DigitalOut led1(LED2);

EventFlags event;
Mutex pomiar_mutex;
Mail<PomiarSensor, 2> comm_mail_box;
Mail<PomiarSensor, 2> pomiar_mail_box;
EventQueue PB_queue(4 * EVENTS_EVENT_SIZE);
EventQueue *zdarzenie = mbed_event_queue();

// definicje watkow

uint8_t net_i[4*1024];
uint8_t net[6*1024];
// uint8_t readt[4*1024];
// uint8_t logic[4*1024];
uint8_t test[1*1024];
uint8_t PBt[1*1024];
uint8_t zda[1*1024];

Thread thread_net_init(osPriorityNormal, 4*1024, net_i, "net");
Thread thread_comm(osPriorityNormal, 6*1024, net, "comm");
Thread thread_read(osPriorityNormal, 2*1024, nullptr, "read");
Thread thread_logic(osPriorityNormal, 2*1024, nullptr, "logic");
Thread thread_test(osPriorityNormal, 1*1024, test, "test");
Thread PB_thread(osPriorityNormal, 1*1024, PBt, "PB");
Thread thread_zdarzenie(osPriorityNormal, 1*1024, zda, "zda");

void test_thread(){
    int duty = 0;   // jednostka: %
    led.period_ms(1);     // czestotliwosc 1kHz.

    while(true){
    for (; duty <= 10; duty += 1) {
            led = duty / 100.0;
            ThisThread::sleep_for(50ms);
            }
        // do 0%, zmniejszenie o 5% co
        for (; duty >= 1; duty -= 1) {
            led = duty / 100.0;
            ThisThread::sleep_for(50ms);
            }
    }
}

void net_init_start(){
    event.clear();
    event.set(NET_INIT_START);
    thread_test.start(&test_thread);
    zdarzenie->call(printf,"udalo sie\n");
    zdarzenie->break_dispatch();
}

int main ()
{

    event.set(IDLE);

    thread_net_init.start(callback(&net_init_thread));
    thread_comm.start(callback(&comm_thread));
    thread_read.start(callback(&read_thread));
    thread_logic.start(callback(&logic_thread));
    PB_thread.start(callback(&PB_queue, &EventQueue::dispatch_forever));
    thread_zdarzenie.start(callback(zdarzenie, &EventQueue::dispatch_once));

    button_push.fall(PB_queue.event(net_init_start));

    zdarzenie->call_every(5s,printf,"Oczekiwanie na nacisniecie niebieskiego przycisku...\n");


    // while (true)
        //  {   
 
        //  }

    thread_net_init.join();
    thread_comm.join();
    thread_read.join();
    thread_logic.join();
    thread_test.join();

    return 0;
}