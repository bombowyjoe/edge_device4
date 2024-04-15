#ifndef COMM_H
#define COMM_H
#include "QUEUES.h"
#include "COMM_INIT.h"




extern NetworkInterface *_defaultSystemNetwork;

void comm_thread();

void extracted_message();

#endif