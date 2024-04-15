// MyQueue.h
#ifndef QUEUES_H
#define QUEUES_H

#define NUMBER_OF_CHARACTERS 1024

#define NET_INIT_START (1UL << 0)
#define NETINIT_COMPLETE (1UL << 1)
#define AZURE_INIT_START (1UL << 2)
#define AZURE_INIT_COMPLETE (1UL << 3)
#define IDLE (1UL << 4)
#define AZURE_ERROR (1UL << 5)

// #include "mbed.h"
#include "Queue.h"
#include "MemoryPool.h"
#include "Mutex.h"
#include "EventFlags.h"
#include "mbed.h"
#include "Json.h"
#include <string>

typedef struct{
    float temp;
    float hum;
    float press;
    int n;
    const char* time;
}  PomiarSensor;

typedef enum{
    DEV_CONNECTED,
    DEV_MEASUREMENT_OK,
    DEV_SENSOR_DISCONNECTED,
    DEV_MEMORY_FULL
} StanUrzadzenia;

typedef struct{

    float temp;
    float hum;
    float press;
    int n;
    const char* time;

}mail_t;

extern NetworkInterface *_defaultSystemNetwork;

// deklaracja kolejki zdarzen
extern EventQueue queue;
extern EventQueue *zdarzenie;

extern Mail<PomiarSensor, 2> pomiar_mail_box;

extern Mail<PomiarSensor, 2> comm_mail_box;

extern Mail<NetworkInterface, 2> inter;

// deklaracja mutexu
extern Mutex pomiar_mutex;

// deklaracja flagi
extern EventFlags event;

#endif 