#ifndef __RELAY_H__
#define __RELAY_H__

#include "zcs.h"

void relay_init(char *channel1_LAN1, char *channel2_LAN1, int port_LAN1, char *channel1_LAN2, char *channel2_LAN2, int port2_LAN2);
void shutdown_relay();

#endif
