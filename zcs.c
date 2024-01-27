#include "multicast.h"

// DEFAULT PORT NUMBERS : 
// SOURCE PORT : 16000
// DESTINATION PORT : 16001

int zcs_init() {
    // TODO : What ports to use? 
    mcast_t *m multicast_init("224.1.1.1", 10000, 10001);
    return 0;
}