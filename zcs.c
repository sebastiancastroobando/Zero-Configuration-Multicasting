#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "multicast.h"
#include "zcs.h"

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT 14500
#define ZCS_ADDR "224.1.1.1"

#define MAX_NAME_LEN 64
#define MAX_AD_DURATION 10 // in seconds
#define MAX_AD_ATTEMPTS 3 // number of attempts to send an ad

typedef struct {
    char name[MAX_NAME_LEN];
    zcs_attribute_t *attributes;
    mcast_t *mcast;
    int num_attributes;
    int isOnline;
} zcs_node_t;

// Node object
static zcs_node_t zcs_node; 

void* discovery(void* arg) {
    // First send a discovery to the multicast group
    // need to encode who is sending the notification
    multicast_send(zcs_node.mcast, "DISCOVERY", strlen("DISCOVERY"));
    char discvoery_buffer[100];
    while(1) {
        // now we need to wait for a response
        multicast_receive(zcs_node.mcast, discvoery_buffer, 100);
    }
}

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
    // When a node comes up (when init is called), it sends a DISCOVERY message
    pthread_t discoveryThread;
    if (pthread_create(&discoveryThread, NULL, &discovery, NULL) != 0) {
        perror("zcs_init: pthread_create");
        return -1;
    }
}

/**
 * @brief Puts the node online
 * @return 0 on success, -1 on failure
*/
int zcs_start(char *name, zcs_attribute_t attr[], int num) {
    // Check if init was called before
    if (zcs_node.mcast == NULL) { 
        perror("zcs_start: init not called yet");
        return -1; 
    }

    // Check if the node name is too long
    if (strlen(name) > MAX_NAME_LEN) {
        perror("zcs_start: node name too long");
        return -1;
    }
    
    strcpy(zcs_node.name, name);
    // TODO : should we add a NULL character at the end? 

    // Allocate the memory necessary for the attributes
    zcs_node.attributes = (zcs_attribute_t *)malloc(sizeof(zcs_attribute_t) * num);

    // Copy the attributes to the node object
    // TODO : should we use memcopy?
    for (int i = 0; i < num; i++) {
        zcs_node.attributes[i] = attr[i];
    }
    zcs_node.num_attributes = num;

    // Send notification to the multicast group

}

/**
 * @brief Puts the node offline
 * @return number of times the ad was posted, 0 on failure (no posting)
*/
int zcs_post_ad(char *ad_name, char *ad_value) {
    // check if node was started
    if (zcs_node.mcast == NULL) { 
        perror("zcs_post_ad: node not started yet");
        return -1; 
    }
    
    
    // Post ad to the multicast group
    // Ad should be posted MAX_AD_ATTEMPTS times over MAX_AD_DURATION seconds
    // For now, we will just implement a loose version of this
    int attempts = 0;
    int duration = 0;
    while (duration < MAX_AD_DURATION) {
        // should we send two messages or one? If one, how do we parse it?
        multicast_send(zcs_node.mcast, ad_name, strlen(ad_name));
        multicast_send(zcs_node.mcast, ad_value, strlen(ad_value));
        attempts++;
        duration++;
        sleep(1);
    }
    return attempts;

}

/**
 * @brief Scan for nodes with a given value for a given attribute. The names of the
 * nodes that match the query are returned in node_names
*/
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen) {
    // TODO
    
}

/**
 * @brief get full list of attributes of a node that is return by a query
*/
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num) {

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

// TODO log function