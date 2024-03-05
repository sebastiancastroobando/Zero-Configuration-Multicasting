#include <stdio.h>

#include "multicast.h"

mcast_t *mcast_LAN1_mrecv; // multicast Channel 1 for LAN1
mcast_t *mcast_LAN1_msend; // multicast Channel 2 for LAN1
mcast_t *mcast_LAN2_mrecv; // multicast Channel 1 for LAN2
mcast_t *mcast_LAN2_msend; // multicast Channel 2 for LAN2

relay_init(char *channel1_LAN1, char *channel2_LAN1, int port_LAN1, char *channel1_LAN2, char *channel2_LAN2, int port2_LAN2) {
    // initialize the multicast groups
    mcast_init(channel1_LAN1, port_LAN1, port_LAN1 + 1);
    mcast_init(channel2_LAN1, port_LAN1 - 1, port_LAN1);
    mcast_init(channel1_LAN2, port2_LAN2, port2_LAN2 + 1);
    mcast_init(channel2_LAN2, port2_LAN2 - 1, port2_LAN2);
}

// The relay is a bridge between two LANs. 
int main() {
    // ask user for channel1, channel2, and port
    char channel1_LAN1[16];
    char channel2_LAN1[16];
    int port_LAN1;

    char channel1_LAN2[16];
    char channel2_LAN2[16];
    int port2_LAN2;

    printf("Relay Configuration\n");
    printf("LAN1 -------------------\n");
    printf("Enter channel1: ");
    scanf("%s", channel1_LAN1);
    printf("Enter channel2: ");
    scanf("%s", channel2_LAN1);
    printf("Enter port: ");
    scanf("%d", &port_LAN1);
    printf("LAN2 -------------------\n");
    printf("Enter channel1: ");
    scanf("%s", channel1_LAN2);
    printf("Enter channel2: ");
    scanf("%s", channel2_LAN2);
    printf("Enter port: ");
    scanf("%d", &port2_LAN2);


    // initialize the multicast groups
    reley_init(channel1_LAN1, channel2_LAN1, port_LAN1, channel1_LAN2, channel2_LAN2, port2_LAN2);
}