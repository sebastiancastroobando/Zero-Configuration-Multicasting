#ifndef __RELAY_H__
#define __RELAY_H__

// LAN_A support
#define LAN_A_CHANNEL1      "224.1.1.1"
#define LAN_A_CHANNEL2      "224.1.1.2"
#define LAN_A_PORT          14500
// LAN_B support
#define LAN_B_CHANNEL1      "224.1.1.3"
#define LAN_B_CHANNEL2      "224.1.1.4"
#define LAN_B_PORT          17500

void relay_init(char *channel1_LAN1, char *channel2_LAN1, int port_LAN1, char *channel1_LAN2, char *channel2_LAN2, int port2_LAN2);
void shutdown_relay();

#endif
