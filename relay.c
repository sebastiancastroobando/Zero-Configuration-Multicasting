#include <stdio.h>

#include "multicast.h"

// multicast groups for LAN1 and LAN2
mcast_t *mcast_LAN1_CHANNEL1_mrecv; // multicast receiver Channel 1 for LAN1
mcast_t *mcast_LAN1_CHANNEL1_msend; // multicast sender Channel 1 for LAN1
mcast_t *mcast_LAN1_CHANNEL2_mrecv; // multicast receiver Channel 2 for LAN1
mcast_t *mcast_LAN1_CHANNEL2_msend; // multicast sender Channel 2 for LAN1
mcast_t *mcast_LAN2_CHANNEL1_mrecv; // multicast receiver Channel 1 for LAN2
mcast_t *mcast_LAN2_CHANNEL1_msend; // multicast sender Channel 1 for LAN2
mcast_t *mcast_LAN2_CHANNEL2_mrecv; // multicast receiver Channel 2 for LAN2
mcast_t *mcast_LAN2_CHANNEL2_msend; // multicast sender Channel 2 for LAN2

/**
 * @details Ports are mirrored between channels in a given LAN, meaning :
 * for an APP in LAN1, it would be :
 * - msend : {channel1, port, port+1}; mrecv: {channel2, port-1, port}. 
 * and for a SERVICE in LAN1, it would be:
 * - msend : {channel2, port, port+1}; mrecv: {channel1, port-1, port}.
 * 
*/

void reley_init(char *channel1_LAN1, char *channel2_LAN1, int port_LAN1, char *channel1_LAN2, char *channel2_LAN2, int port2_LAN2) {
    // initialize the multicast groups for LAN1
    mcast_LAN1_CHANNEL1_mrecv = mcast_init(channel1_LAN1, port_LAN1-1, port_LAN1);
    mcast_LAN1_CHANNEL1_msend = mcast_init(channel1_LAN1, port_LAN1, port_LAN1+1);
    mcast_LAN1_CHANNEL2_mrecv = mcast_init(channel2_LAN1, port_LAN1-1, port_LAN1);
    mcast_LAN1_CHANNEL2_msend = mcast_init(channel2_LAN1, port_LAN1, port_LAN1+1);

    // initialize the multicast groups for LAN2
    mcast_LAN2_CHANNEL1_mrecv = mcast_init(channel1_LAN2, port2_LAN2-1, port2_LAN2);
    mcast_LAN2_CHANNEL1_msend = mcast_init(channel1_LAN2, port2_LAN2, port2_LAN2+1);
    mcast_LAN2_CHANNEL2_mrecv = mcast_init(channel2_LAN2, port2_LAN2-1, port2_LAN2);
    mcast_LAN2_CHANNEL2_msend = mcast_init(channel2_LAN2, port2_LAN2, port2_LAN2+1);

    // setup receiver multicast objects
    mcast_setup_recv(mcast_LAN1_CHANNEL1_mrecv);
    mcast_setup_recv(mcast_LAN1_CHANNEL2_mrecv);
    mcast_setup_recv(mcast_LAN2_CHANNEL1_mrecv);
    mcast_setup_recv(mcast_LAN2_CHANNEL2_mrecv);

    
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