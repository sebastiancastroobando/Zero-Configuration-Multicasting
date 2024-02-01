#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <err.h>

#include "multicast.h"
#include "zcs.h"

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT			14500
#define ZCS_CHNL_SEND		"224.1.1.1"
#define ZCS_CHNL_RECV		"224.1.1.2"

#define MAX_NAME_LEN		64
#define MAX_AD_DURATION		10 // in seconds
#define MAX_AD_ATTEMPTS		3 // number of attempts to send an ad
#define HEARTBEAT_INTERVAL	1 // in seconds
#define BUF_SIZE			100
#define MAX_SIZE			10

#define ZCS_APP_TYPE		1
#define ZCS_SERVICE_TYPE	2

typedef struct {
	char name[MAX_NAME_LEN];
    int type;
    zcs_attribute_t *attributes;
    mcast_t *msend;
	mcast_t *mrecv;
    int num_attributes;
    int isOnline;
} zcs_node_t;

// Node object
static zcs_node_t zcs_node; 

void zcs_multicast_send(char *msg) {
	multicast_send(zcs_node.msend, msg, strlen(msg));
	// resend if not received
	while (multicast_check_receive(zcs_node.msend) == 0) {
		printf("resend\n");
		multicast_send(zcs_node.msend, msg, strlen(msg));
	}
}

int discovery() {
    // First send a discovery to the multicast group
    // need to encode who is sending the notification
    // Ex of message: "DISCOVERY:node_name"
    char discovery_msg[BUF_SIZE];
	const char delim = ':';
	char *token;
	char *received_data[MAX_SIZE];
	int i;

    strcpy(discovery_msg, "DISCOVERY:");
	strcat(discovery_msg, "APP:");
	strcat(discovery_msg, zcs_node.name);

    char discovery_buffer[BUF_SIZE];

	zcs_multicast_send(discovery_msg);

    // keep processing messages as they come in
    while (multicast_check_receive(zcs_node.msend) > 0) {
	    // now we need to wait for a response
	    // what happens if we receive two messages at the same time?
	    // Ex of notification: "NOTIFICATION:node_name"
	    multicast_receive(zcs_node.mrecv, discovery_buffer, BUF_SIZE);
	    if (strstr(discovery_buffer, "NOTIFICATION:") != NULL) {
		    // The NOTIFICATION will have full information about the service. 
		    // how do we encode the attributes?
		    // Ex of notification: 
		    // "NOTIFICATION:type:name:attr1:val1:attr2:val2:attr3:val3"
			token = strtok(str, &delim);
			i = 0;
			while (token !=	NULL) {
				if (i == MAX_SIZE)
					break;
				strcpy(received_data[i++], token);
				token = strtok(NULL, &delim);
			}
			if (strcmp(received_data[1], "APP") == 0)
				continue;
			// parse the rest of the received data
		}
	}
	return 0;
}

int notification() {
	// FORMAT: "NOTIFICATION:type:name:attr1:val1:attr2:val2:attr3:val3"
	char msg[BUF_SIZE];
	strcpy(msg, "NOTIFICATION:");
	strcat(msg, "SERVICE:");
	strcat(msg, zcs_node.name);
	for (int i = 0; i < zcs_node.num_attributes; i++) {
		strcat(msg, &zcs_node->attributes[i].attr_name);
		strcat(msg, ":");
		strcat(msg, &zcs_node->attributes[i].value);
		strcat(msg, ":");
	}

	zcs_multicast_send(msg);

	return 0;
}

// heartbeat should probably also just be an overall listener
// for DISCOVERY for example, or just any incoming messages
void* listener(void* arg) {
	char heartbeat_msg[BUF_SIZE];
	char msg[BUF_SIZE];
	strcpy(heartbeat_msg, "HEARTBEAT:");
	strcat(heartbeat_msg, zcs_node.name);
    // TODO
    while(1) {
        // send a heartbeat to the multicast group, if node is a service
        // need to encode who is sending the notification
        // Ex of message: "HEARTBEAT:node_name"
		if (zcs_node.type == ZCS_SERVICE_TYPE) {

			zcs_multicast_send(heartbeat_msg);
			sleep(HEARTBEAT_INTERVAL);

			// node receives an incoming message
			if (multicast_check_receive(zcs_node.msend) > 0) {
				multicast_receive(zcs_node.mrecv, msg, BUF_SIZE);
				if (strstr(msg, "DISCOVERY:") != NULL) {
					// arrange self data as specified earlier
					// "NOTIFICATION:type:name:attr1:val1:attr2:val2:attr3:val3"
					// call the notification function
					notification();
				}
			}
		}
    }
}

/**
 * @brief Setup the parameters and initializations of necesasry components
 * @return 0 on success, -1 on failure
*/
int zcs_init(int type) {
    // Initialize multicast
	// REMARK: since we now know that we need two comm channels,
	// I don't think node should set a valid listen port if its
	// sending something and conversely should not set a valid send
	// port if its listening
	int channel1, channel2;
	if (type == ZCS_APP_TYPE) {
		channel1 = ZCS_CHNL_SEND;
		channel2 = ZCS_CHNL_RECV;
	} else if (type == ZCS_SERVICE_TYPE) {
		channel1 = ZCS_CHNL_RECV;
		channel2 = ZCS_CHNL_SEND;
	}
    mcast_t *msend = multicast_init(channel1, ZCS_PORT, ZCS_PORT + 1);
	mcast_t *mrecv = multicast_init(channel2, ZCS_PORT + 1, ZCS_PORT);
    if (!msend || !mrecv) {
		perror("zcs_init: multicast_init\n");
		return -1;
	}

	multicast_setup_recv(mrecv);

	zcs_node.type = type;
    zcs_node.msend = msend; // save the multicast object to the node object
	zcs_node.mrecv = mrecv;
    zcs_node.isOnline = 1; // set the node to offline
    return 0;
    // When a node comes up (when init is called), it sends a DISCOVERY message
    // I'm pretty sure this is called on start, since init only initializes the node
	// start actually puts the node online
	/*pthread_t discoveryThread;
    if (pthread_create(&discoveryThread, NULL, &discovery, NULL) != 0) {
        perror("zcs_init: pthread_create\n");
        return -1;
    }*/
}

/**
 * @brief Puts the node online
 * @return 0 on success, -1 on failure
*/
int zcs_start(char *name, zcs_attribute_t attr[], int num) {
    // Check if init was called before
    if (!zcs_node.msend || !zcs_node.mrecv) {
        perror("zcs_start: init not called yet\n");
        return -1; 
    }

    // Check if the node name is too long
    if (strlen(name) > MAX_NAME_LEN) {
        perror("zcs_start: node name too long\n");
        return -1;
    }
    
    // TODO : should we add a NULL character at the end?
	// RESOLVED: handout says that name is assumed to already be NULL terminated 
    strcpy(zcs_node.name, name);

    // Allocate the memory necessary for the attributes
    zcs_node.attributes = (zcs_attribute_t *)malloc(sizeof(zcs_attribute_t) * num);

    // Copy the attributes to the node object
    memcpy(zcs_node.attributes, attr, num * sizeof(zcs_attribute_t));
    zcs_node.num_attributes = num;

	if (zcs_node.type == ZCS_APP_TYPE) {
		discovery();
	} else if (zcs_node.type == ZCS_SERVICE_TYPE) {
		notification();
	} else {
		perror("zcs_start: wrong node type\n");
		return -1;
	}

    // turn the node on, aka listen to activity
	// both nodes would listen for DISCOVERY, query, or ad
	// heartbeat is included if it is a service node
    pthread_t listenerThread;
    if (pthread_create(&listenerThread, NULL, &listener, NULL) != 0) {
        perror("zcs_start: pthread_create\n");
        return -1;
    }
}

/**
 * @brief Puts the node offline
 * @return number of times the ad was posted, 0 on failure (no posting)
*/
int zcs_post_ad(char *ad_name, char *ad_value) {
    // check if node was started
    if (!zcs_node.msend || !zcs_node.mrecv) {
        perror("zcs_post_ad: node not started yet\n");
        return -1; 
    }
    
    
    // Post ad to the multicast group
    // Ad should be posted MAX_AD_ATTEMPTS times over MAX_AD_DURATION seconds
    // For now, we will just implement a loose version of this
	// REMARK: should we use zcs_multicast_send?
    int attempts = 0;
    int duration = 0;
    while (duration < MAX_AD_DURATION) {
        // should we send two messages or one? If one, how do we parse it?
        multicast_send(zcs_node.msend, ad_name, strlen(ad_name));
        multicast_send(zcs_node.msend, ad_value, strlen(ad_value));
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
    multicast_destroy(zcs_node.msend);
	multicast_destroy(zcs_node.mrecv);
    // set the node to offline
    zcs_node.isOnline = 0;
    return 0;
}

// TODO log function
