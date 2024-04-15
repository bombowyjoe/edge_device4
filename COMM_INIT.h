#ifndef COMM_INIT_H
#define COMM_INIT_H

#include "certs.h"
#include "iothub.h"
#include "iothub_client_options.h"
#include "iothub_device_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/xlogging.h"
#include "iothubtransportmqtt.h"
#include "NTPClient.h"
#include "QUEUES.h"

void net_init_thread();

#endif