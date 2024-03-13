#ifndef __RELAY_H__
#define __RELAY_H__

// LAN_A support
#define LAN_A_CHANNEL1      "224.1.1.1"
#define LAN_A_CHANNEL2      "224.1.1.2"
#define LAN_A_PORT1         14500
#define LAN_A_PORT2         16500
// LAN_B support
#define LAN_B_CHANNEL1      "224.1.1.3"
#define LAN_B_CHANNEL2      "224.1.1.4"
#define LAN_B_PORT1         17500
#define LAN_B_PORT2         18500

void relay_init(char *channel1_LAN_A, char *channel2_LAN_A, int LAN_A_port1, int LAN_A_port2, char *channel1_LAN_B, char *channel2_LAN_B, int LAN_B_port1, int LAN_B_port2);
void shutdown_relay();

#endif
