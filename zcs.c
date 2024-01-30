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
#define HEARTBEAT_INTERVAL 1 // in seconds

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
    // Ex of message: "DISCOVERY:node_name"
    char discovery_msg[100]; 
    strcpy(discovery_msg, "DISCOVERY:");
    strcat(discovery_msg, zcs_node.name);
    multicast_send(zcs_node.mcast, discovery_msg, strlen(discovery_msg));

    char discovery_buffer[100];
    while(1) {
        // now we need to wait for a response
        // what happens if we receive two messages at the same time?
        // Ex of notification: "NOTIFICATION:node_name"
        multicast_receive(zcs_node.mcast, discovery_buffer, 100);
        if (strstr(discovery_buffer, "NOTIFICATION:") != NULL) {
            // The NOTIFICATION will have full information about the service. 
            // how do we encode the attributes?
            // Ex of notification: 
            // "NOTIFICATION:node_name:attr1:val1:attr2:val2:attr3:val3"
        }
    }
}

void* heartbeat(void* arg) {
    // TODO
    while(1) {
        // send a heartbeat to the multicast group
        // need to encode who is sending the notification
        // Ex of message: "HEARTBEAT:node_name"
        char heartbeat_msg[100];
        strcpy(heartbeat_msg, "HEARTBEAT:");
        strcat(heartbeat_msg, zcs_node.name);
        multicast_send(zcs_node.mcast, heartbeat_msg, strlen(heartbeat_msg));
        sleep(HEARTBEAT_INTERVAL);
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
    
    // TODO : should we add a NULL character at the end? 
    strcpy(zcs_node.name, name);

    // Allocate the memory necessary for the attributes
    zcs_node.attributes = (zcs_attribute_t *)malloc(sizeof(zcs_attribute_t) * num);

    // Copy the attributes to the node object
    memcpy(zcs_node.attributes, attr, num * sizeof(zcs_attribute_t));
    zcs_node.num_attributes = num;

    // Send notification to the multicast group
    pthread_t heartbeatThread;
    if (pthread_create(&heartbeatThread, NULL, &heartbeat, NULL) != 0) {
        perror("zcs_start: pthread_create");
        return -1;
    }
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