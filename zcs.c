#include <unistd.h>
#include <stdio.h>

#include "multicast.h"
#include "zcs.h"

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT 14500
#define ZCS_ADDR "224.1.1.1"

typedef struct {
    char name[64];
    zcs_attribute_t *attributes;
    int num_attributes;
    int isOnline;
} zcs_node_t;

/**
 * @brief Setup the parameters and initializations of necesasry components
 * @return 0 on success, -1 on failure
*/
int zcs_init() {
    // Multithreading here?
    // Initialize multicast
    mcast_t *m = multicast_init(ZCS_ADDR, ZCS_PORT, ZCS_PORT);
    if (m == NULL) { return -1;}
    return 0;
}

/**
 * @brief Puts the node online
 * @param name node name (ASCII, NULL terminated)
 * @param attr key-value pairs, attributes of the node
 * @param num number of attributes
 * @return 0 if node start successfully, -1 if start before init
*/
int zcs_start(char *name, zcs_attribute_t attr[], int num) {
    // check if init is called

}
