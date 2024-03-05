#include <stdio.h>

#include "multicast.h"

int init_relay() {
    // initialize the multicast groups
    int rv;
    rv = zcs_init(ZCS_APP_TYPE, channel1_LAN1, channel2_LAN1, port_LAN1);
    if (rv < 0) {
        printf("Error initializing ZCS for LAN1\n");
        return -1;
    }
    rv = zcs_init(ZCS_APP_TYPE, channel1_LAN2, channel2_LAN2, port2_LAN2);
    if (rv < 0) {
        printf("Error initializing ZCS for LAN2\n");
        return -1;
    }
    return 0;

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

}