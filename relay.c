#include <stdio.h>

#include "multicast.h"



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