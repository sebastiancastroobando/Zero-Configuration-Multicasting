#include <unistd.h>
#include <stdio.h>

#include "multicast.h"
#include "zcs.h"

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT 14500
#define ZCS_ADDR "224.1.1.1"
#define MAX_NAME_LEN 64

typedef struct {
    char name[MAX_NAME_LEN];
    zcs_attribute_t *attributes;
    mcast_t *mcast;
    int num_attributes;
    int isOnline;
} zcs_node_t;

// Node object
static zcs_node_t zcs_node; 

/**
 * @brief Setup the parameters and initializations of necesasry components
 * @return 0 on success, -1 on failure
*/
int zcs_init() {
    // Initialize multicast
    mcast_t *m = multicast_init(ZCS_ADDR, ZCS_PORT, ZCS_PORT);
    if (m == NULL) { return -1;}
    zcs_node.mcast = m; // save the multicast object to the node object
    zcs_node.isOnline = 1; // set the node to offline
    return 0;
}

/**
 * @brief Puts the node online
 * @return 0 on success, -1 on failure
*/
int zcs_start(char *name, zcs_attribute_t attr[], int num) {
    // check if init was called
    if (zcs_node.mcast == NULL) { 
        perror("zcs_start: init not called yet");
        return -1; 
    }

    // Copy name to the node object
    if (strlen(name) > MAX_NAME_LEN) {
        perror("zcs_start: node name too long");
        return -1;
    }
    strcpy(zcs_node.name, name);
    // TODO : is this necessary?
    // Ensure that the string is null-terminated. 
    zcs_node.name[sizeof(zcs_node.name) - 1] = '\0';

    // Allocate the memory necessary for the attributes
    zcs_node.attributes = (zcs_attribute_t *)malloc(sizeof(zcs_attribute_t) * num);

    // Copy the attributes to the node object
    // TODO : should we use memcopy?
    for (int i = 0; i < num; i++) {
        zcs_node.attributes[i] = attr[i];
    }
    zcs_node.num_attributes = num;
}

int zcs_shutdown() {
    // free the memory allocated for the attributes
    free(zcs_node.attributes);
    // free the memory allocated for the multicast object
    multicast_destroy(zcs_node.mcast);
    // set the node to offline
    zcs_node.isOnline = 0;
    return 0;
}