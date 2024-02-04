#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <err.h>
#include <math.h>
#include <stdlib.h>

#include "multicast.h"
#include "zcs.h"

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT			14500
#define ZCS_CHANNEL1		"224.1.1.1"
#define ZCS_CHANNEL2		"224.1.1.2"

#define MAX_NAME_LEN		64
#define MAX_AD_DURATION		10 // in seconds
#define MAX_AD_ATTEMPTS		3 // number of attempts to send an ad
#define HEARTBEAT_INTERVAL	1 // in seconds
#define BUF_SIZE			1000 // TODO : 100 seems like too little, let's talk about this.
#define MAX_SIZE			10 // max number of nodes
#define MAX_MSG_SIZE		20 // max number of {attr_name, value} pairs

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

typedef struct {
	zcs_node_t nodes[MAX_SIZE];
	int num_nodes;
} zcs_reg_t;

// Node object
static zcs_node_t zcs_node;
pthread_t *heartbeatThread;
pthread_t *notificationThread;
zcs_reg_t local_reg;
// static = only accessible from the library itself
// volatile = ensures that compiler will not "optimize" this value
// and every access will fetch the lastest value
static volatile int detatchThreadServicesSignal = 1;

// @check : Good. No need to change.
void zcs_multicast_send(char *msg) {
	multicast_send(zcs_node.msend, msg, strlen(msg));
	// resend if not received
	while (multicast_check_receive(zcs_node.msend) == 0) {
		printf("resend\n");
		multicast_send(zcs_node.msend, msg, strlen(msg));
	}
}

// @check : @Denis, what do you think of this way to handle the received message?
void split_key_value(char *str, char **key, char **value) {
	*key = strtok(str, ":");
	*value = strtok(NULL, ":");
}

// @check : I don't like the parsing of the message here.
int make_reg_entry(char *data[], int dsize) {
	// data[0] = "msgType:NOTIFICATION"
	// data[1] = "nodeName:node_name"
	// ...
	zcs_node_t *node = (zcs_node_t*) malloc(sizeof(zcs_node_t));
	if (!node) {
		perror("make_reg_entry: bad malloc\n");
		exit(-1);
	}

	// TODO : only an app will call this function, therefore we can assume that the type is a service?
	node->type = ZCS_SERVICE_TYPE;
	// parse the node name
	char *key, *value;
	split_key_value(data[1], &key, &value);
	// put the node name in the node object
	strcpy(node->name, value);
	// fill the rest of the node object
	node->isOnline = 0;
	node->num_attributes = dsize - 2;
	node->attributes = (zcs_attribute_t*) malloc(sizeof(zcs_attribute_t) * node->num_attributes);
	
	if (!node->attributes) {
		perror("make_reg_entry: bad malloc\n");
		exit(-1);
	}

	for (int i = 2; i < dsize; i++) {
		// fill the attributes
		split_key_value(data[i], &key, &value);
		strcpy(node->attributes[i - 2].attr_name, key);
		strcpy(node->attributes[i - 2].value, value);
	} 
	// fill local registry
	local_reg.nodes[local_reg.num_nodes++] = *node;
	return 0;
}

// @check : I don't like the sending message here.
int discovery() {
    // First send a discovery to the multicast group
    // need to encode who is sending the notification

    char discovery_msg[BUF_SIZE];
	const char delim = ';'; 
	char *token;
	char *received_data[MAX_MSG_SIZE];
	int dsize;

	strcpy(discovery_msg, "msgType:DISCOVERY;nodeName:");
	strcat(discovery_msg, zcs_node.name);
	strcat(discovery_msg, ";"); // delimiter will mark the end of the message

    char discovery_buffer[BUF_SIZE];

	zcs_multicast_send(discovery_msg);

    // @Check this, is it going to wait for all the messages to be received?
    while (multicast_check_receive(zcs_node.msend) > 0) {
	    // We will receive a message from at most 10 services nodes
	
	    multicast_receive(zcs_node.mrecv, discovery_buffer, BUF_SIZE);
		if (strstr(discovery_buffer, "msgType:NOTIFICATION;") != NULL) {
		    // the message is a notification, we need to parse it
			char *str;
			token = strtok(discovery_buffer, &delim);
			dsize = 0;
			while (token != NULL) {
				// check if we have reached the maximum size of the received data
				if (dsize == MAX_MSG_SIZE)
					// TODO : should we notify the user that we have reached the maximum size of paramaters?
					break;
				// allocate memory for the received data
				received_data[dsize] = (char*) malloc(strlen(token) * sizeof(char));
				// check if the allocation was successful
				if (!received_data[dsize]) {
					perror("discover: bad malloc\n");
					exit(-1);
				}
				// copy the token to the received data
				strcpy(received_data[dsize++], token);
				token = strtok(NULL, &delim);
			}
			// make a registry entry
			make_reg_entry(received_data, dsize);
		}
	}
	return 0;
}

// @check : should this be void?
void* notification(void* arg) {
	// FORMAT: "msgType:NOTIFICATION;nodeName:node_name;isOnline:0;attr1:val1;attr2:val2;attr3:val3"
	char msg[BUF_SIZE];
	strcpy(msg, "msgType:NOTIFICATION;");
	strcat(msg, "nodeName:");
	strcat(msg, zcs_node.name);
	strcat(msg, ";");
	for (int i = 0; i < zcs_node.num_attributes; i++) {
		strcat(msg, zcs_node.attributes[i].attr_name);
		strcat(msg, ":");
		strcat(msg, zcs_node.attributes[i].value);
		strcat(msg, ";");
	}

	// send the notification to the multicast group
	zcs_multicast_send(msg);

	// wait for incoming messages DISCOVERY messages
	while(detatchThreadServicesSignal) {
		if (multicast_check_receive(zcs_node.msend) > 0) {
			multicast_receive(zcs_node.mrecv, msg, BUF_SIZE);
			if (strstr(msg, "msgType:DISCOVERY;") != NULL) {
				zcs_multicast_send(msg);
			}
		}
	}
	return NULL;
}

// heartbeat should probably also just be an overall listener
// for DISCOVERY for example, or just any incoming messages
void* heartbeat(void* arg) {
	char heartbeat_msg[BUF_SIZE];
	char msg[BUF_SIZE];
	strcpy(heartbeat_msg, "msgType:HEARTBEAT;nodeName:");
	strcat(heartbeat_msg, zcs_node.name);
	strcat(heartbeat_msg, ";");

    while(detatchThreadServicesSignal) {
        // example of heartbeat : "msgType:HEARTBEAT;nodeName:node_name"
		// sanity check... although only a service node would call this function
		printf("\n ==loop== \n");
		if (zcs_node.type == ZCS_SERVICE_TYPE) {
			printf("enter here\n");
			zcs_multicast_send(heartbeat_msg);
			//sleep(HEARTBEAT_INTERVAL);
			printf("\n ==end sleep== \n");
		}
    }
	printf("\n ==end loop== \n");
}

/**
 * @brief Setup the parameters and initializations of necesasry components
 * @return 0 on success, -1 on failure
*/
int zcs_init(int type) {
    // Initialize multicast groups
	// send_channel and recv_channel should be char* and not int
	char *send_channel, *recv_channel;
	if (type == ZCS_APP_TYPE) {
		send_channel = ZCS_CHANNEL1;
		recv_channel = ZCS_CHANNEL2;
	} else if (type == ZCS_SERVICE_TYPE) {
		send_channel = ZCS_CHANNEL2;
		recv_channel = ZCS_CHANNEL1;
	}
	
	// create the multicast objects, one for sending and one for receiving
	// For sending, only the destination port is needed
	// For receiving, only the source port is needed
    mcast_t *msend = multicast_init(send_channel, ZCS_PORT, ZCS_PORT + 1);
	mcast_t *mrecv = multicast_init(recv_channel, ZCS_PORT - 1, ZCS_PORT);
    
	// check if the multicast objects were created successfully
	if (!msend || !mrecv) {
		perror("zcs_init: multicast_init\n");
		return -1;
	}

	// setup the receive multicast object
	multicast_setup_recv(mrecv);

	zcs_node.type = type;
    zcs_node.msend = msend; // save the multicast object to the node object
	zcs_node.mrecv = mrecv;
    zcs_node.isOnline = 1; // set the node to offline

    // An app needs to send a discovery message here to the multicast group
	// because it will not call zcs_start. 
	if (type == ZCS_APP_TYPE) {
		discovery();
	}

	return 0;
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

	// check for ":" and ";" in the attributes. These are our delimiters
	for (int i = 0; i < num; i++) {
		if (strchr(attr[i].attr_name, ':') != NULL || strchr(attr[i].attr_name, ';') != NULL) {
			perror("zcs_start: bad attribute name, contains ':' or ';'\n");
			return -1;
		}
		if (strchr(attr[i].value, ':') != NULL || strchr(attr[i].value, ';') != NULL) {
			perror("zcs_start: bad attribute name, contains ':' or ';'\n");
			return -1;
		}
	}

	// for services nodes, check if number of attributes are more than MAX_MSG_SIZE?
	// should we do -3 to account for the nodeName, msgType, and isOnline?
	if (num > MAX_MSG_SIZE - 3) {
		perror("zcs_start: too many attributes\n");
		return -1;
	}
    
	// Copy the name to the node object
    strcpy(zcs_node.name, name);

    // Allocate the memory necessary for the attributes
    zcs_node.attributes = (zcs_attribute_t *) malloc(sizeof(zcs_attribute_t) * num);

    // Copy the attributes to the node object
    memcpy(zcs_node.attributes, attr, num * sizeof(zcs_attribute_t));
    zcs_node.num_attributes = num;

	zcs_node.isOnline = 0;

	// no need to call the notification here as the thread will do that

	// create a listener thread that will listen for incoming DISCOVERY messages
	// and send a notification
	printf("we got here!\n");
	notificationThread = (pthread_t*) malloc(sizeof(pthread_t));
	if (pthread_create(notificationThread, NULL, &notification, NULL) || !notificationThread) {
		perror("zcs_start: pthread_create - notificatin thread not created\n");
		return -1;
	}
	// we need to detach the thread from the execution of this function
	pthread_detach(*notificationThread);

	// Start the service status thread which will send heartbeats
	// and listen for incoming messages
	heartbeatThread = (pthread_t*) malloc(sizeof(pthread_t));

	if (pthread_create(heartbeatThread, NULL, &heartbeat, NULL) || !heartbeatThread) {
		perror("zcs_start: pthread_create - discovery thread not created\n");
		return -1;
	}
	// we need to detach the thread from the execution of this function
	pthread_detach(*heartbeatThread);

	return 0;
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
	int attempts = 0;
	int ad_rate = ceil(MAX_AD_DURATION / MAX_AD_ATTEMPTS);

	// we will send the ad at ad_rate intervals
	while (attempts < MAX_AD_ATTEMPTS) {
		// send the ad
		// how should we encode the ad?
		// FORMAT: "msgType:AD;nodeName:node_name;adName:ad_name;adValue:ad_value" ? 
		// another idea? 
		char ad_msg[BUF_SIZE];
		strcpy(ad_msg, "msgType:AD;nodeName:");
		strcat(ad_msg, zcs_node.name);
		strcat(ad_msg, ";adName:");
		strcat(ad_msg, ad_name);
		strcat(ad_msg, ";adValue:");
		strcat(ad_msg, ad_value);
		strcat(ad_msg, ";");
		// send the ad
		zcs_multicast_send(ad_msg);
		attempts++;
		sleep(ad_rate);
	}

    return attempts;
}

/**
 * @brief Listen for ads with a given name
 * @return 0 on success, -1 on failure
*/
int zcs_listen_ad(char *name, zcs_cb_f cback) {
	// name is the name of the node that we want to listen to
	// cback is the callback function that will be called when an ad is received
	// FORMAT: "msgType:AD;nodeName:node_name;adName:ad_name;adValue:ad_value"

	char msg[BUF_SIZE];
	char ad_name[BUF_SIZE];
	char ad_value[BUF_SIZE];
	while (1) {
		if (multicast_check_receive(zcs_node.msend) > 0) {
			multicast_receive(zcs_node.mrecv, msg, BUF_SIZE);
			if (strstr(msg, "msgType:AD;") != NULL) {
				// the message is an ad, we need to check 
				// it nodeName matches the name we are listening to
				// if not, ignore the message
				char *str;
				str = strtok(msg, ";");
				// first check if the nodeName matches the name we are listening to
				char *key, *value;
				split_key_value(str, &key, &value);
				if (strcmp(key, "nodeName") == 0 && strcmp(value, name) == 0) {
					// grab the adName and adValue
					str = strtok(NULL, ";"); // skip to the adName
					split_key_value(str, &key, &value);
					if (strcmp(key, "adName") == 0) {
						strcpy(ad_name, value);
					}
					str = strtok(NULL, ";"); // skip to the adValue
					split_key_value(str, &key, &value);
					if (strcmp(key, "adValue") == 0) {
						strcpy(ad_value, value);
					}
					// call the callback function
					cback(ad_name, ad_value);
				}
				else {
					// ignore the message, name does not match
					continue;
				}
			}
		}
	}
	return 0;
}

/**
 * @brief Scan for nodes with a given value for a given attribute. The names of the
 * nodes that match the query are returned in node_names
*/
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen) {
    // TODO
	return 0;
}

/**
 * @brief get full list of attributes of a node that is return by a query
*/
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num) {
	// check if the node is in the local registry
	for (int i = 0; i < local_reg.num_nodes; i++) {
		if (strcmp(local_reg.nodes[i].name, name) == 0) {
			// copy the attributes to the attr array
			memcpy(attr, local_reg.nodes[i].attributes, local_reg.nodes[i].num_attributes * sizeof(zcs_attribute_t));
			*num = local_reg.nodes[i].num_attributes;
			// return the number of attributes
			return *num;
		}
	}
	// node not found
	return -1;
}



/**
 * @brief Log the message to the console
*/
void zcs_log() {
	// TODO
	// we have to maintain some sort of log which would look like this: 
	// node_name:UP->DOWN->UP->UP->DOWN->UP ...
	// see thread#23 for more details https://edstem.org/us/courses/52052/discussion/4170350
}

int zcs_shutdown() {
	// free the memory allocated for the threads
	// pthread_join(*heartbeatThread, NULL);
	// pthread_cancel(*heartbeatThread);
	detatchThreadServicesSignal = 0;
	// TODO : we need to give some time to the thread to terminate gracefully and then free mem
	// how long should it be? 
	// free(heartbeatThread);
	// pthread_join(*notificationThread, NULL);
	// pthread_cancel(*notificationThread);
	// free(notificationThread);

    // free the memory allocated for the attributes
    free(zcs_node.attributes);
    // free the memory allocated for the multicast object
    multicast_destroy(zcs_node.msend);
	multicast_destroy(zcs_node.mrecv);
    // set the node to offline
    zcs_node.isOnline = 0;
    return 0;
}

