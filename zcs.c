#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <err.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

#include "multicast.h"
#include "zcs.h"

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT			14500
#define ZCS_CHANNEL1		"224.1.1.1"
#define ZCS_CHANNEL2		"224.1.1.2"

#define MAX_NAME_LEN		64
#define MAX_AD_DURATION		10 // in seconds
#define MAX_AD_ATTEMPTS		3 // number of attempts to send an ad
#define HEARTBEAT_INTERVAL	5 // in seconds
#define BUF_SIZE			1000 // TODO : 100 seems like too little, let's talk about this.
#define MAX_SIZE			10 // max number of nodes
#define MAX_MSG_SIZE		20 // max number of {attr_name, value} pairs
#define LOG_SIZE			100 // max number of log entries per node (UP/DOWN)

typedef struct {
	char name[MAX_NAME_LEN];
    int type;
    zcs_attribute_t *attributes;
    mcast_t *msend;
	mcast_t *mrecv;
    int num_attributes;
	// @TODO: should we do a struct for the log?
	time_t log[LOG_SIZE];
    int oldest_log_index;
    int log_count;
} zcs_node_t;

typedef struct {
	zcs_node_t nodes[MAX_SIZE];
	int num_nodes;
} zcs_reg_t;


// Node object
static zcs_node_t zcs_node;
pthread_t *heartbeatThread;
pthread_t *notificationThread;
pthread_t *appThread;
zcs_reg_t local_reg;

// ------------------- Helper functions (Debugging) -------------------
// @Description : Print the node object
void print_node(zcs_node_t *node) {
	printf("name: %s\n", node->name);
	printf("type: %d\n", node->type);
	printf("num_attributes: %d\n", node->num_attributes);
	for (int i = 0; i < node->num_attributes; i++) {
		printf("attr_name: %s\n", node->attributes[i].attr_name);
		printf("value: %s\n", node->attributes[i].value);
	}
}
// @Description : Print the local registry
void print_local_reg(zcs_reg_t *reg) {
	for (int i = 0; i < reg->num_nodes; i++) {
		print_node(&reg->nodes[i]);
	}
}
// ---------------------------------------------------------------------

// --------------- Helper functions (Memory Management) ----------------
// @Description : Free the memory allocated for the attributes
void free_node_attributes(zcs_node_t *node) {
	// free the memory allocated for the attributes and NULL the pointers
    if (node->attributes != NULL) {
        for (int i = 0; i < node->num_attributes; i++) {
            free(node->attributes[i].attr_name); 
            node->attributes[i].attr_name = NULL; 

            free(node->attributes[i].value); 
            node->attributes[i].value = NULL; 
        }
        free(node->attributes);
        node->attributes = NULL; 
    }
}

// @TODO: Verify there is no other memory that needs to be freed

// ---------------------------------------------------------------------

//@Description : Add timestamp to the log
void add_log(zcs_node_t *node) {
	if (node->log_count == LOG_SIZE) {
		// we have reached the max number of log entries
		// we need to remove the oldest log entry
		node->log[node->oldest_log_index] = time(NULL);
		node->oldest_log_index = (node->oldest_log_index + 1) % LOG_SIZE;
	} else {
		node->log[node->log_count++] = time(NULL);
	}
}


// @Description : Check if the node is in the local registry
// @Return : -1 if not found, index of the node if found
int find_node(zcs_reg_t *reg, char *name) {
	for (int i = 0; i < reg->num_nodes; i++) {
		if (strcmp(reg->nodes[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

// @Description : Send a message to the multicast group and wait for a response
// @TODO: Should we remove this function? It does not fit with our current design.
void zcs_multicast_send(char *msg) {
	multicast_send(zcs_node.msend, msg, strlen(msg)+1);
	// resend if not received
	while (multicast_check_receive(zcs_node.mrecv) == 0) {
		printf("resend\n");
		multicast_send(zcs_node.msend, msg, strlen(msg)+1);
	}
}

// @Description : Split the key and value of a key-value pair
void split_key_value(char *str, char **key, char **value) {
	// split the string at the colon
	// Format : "key:value"
	*key = strtok(str, ":");
	*value = strtok(NULL, ":");
}

// @Description : Make a registry entry for a node
int make_reg_entry(char *data[], int dsize) {
	// example of data:
	// data[0] = "msgType:NOTIFICATION"
	// data[1] = "nodeName:node_name"
	// data[2] = "attr1:val1"

	zcs_node_t *node = (zcs_node_t*) malloc(sizeof(zcs_node_t));
	if (!node) {
		perror("make_reg_entry: bad malloc\n");
		exit(1);
	}

	// TODO : only an app will call this function, therefore we can assume that the type is a service?
	node->type = ZCS_SERVICE_TYPE;
	// parse the node name
	char *key, *value;

	split_key_value(data[1], &key, &value);
	// put the node name in the node object
	strcpy(node->name, value);
	// fill the rest of the node object
	node->num_attributes = dsize - 2;
	// allocate memory for the attributes
	node->attributes = (zcs_attribute_t*) malloc((dsize - 2) * sizeof(zcs_attribute_t));
	if (!node->attributes) {
		perror("make_reg_entry: bad malloc\n");
		exit(1);
	}
	// fill the attributes
	for (int i = 2; i < dsize; i++) {
		split_key_value(data[i], &key, &value);

		// Allocate memory for attr_name and check for allocation success
		node->attributes[i-2].attr_name = (char *)malloc(strlen(key) + 1); // +1 for null terminator
		if (node->attributes[i-2].attr_name == NULL) {
			perror("Failed to allocate memory for attr_name\n");
			exit(1);
		}

		// Allocate memory for value and check for allocation success
		node->attributes[i-2].value = (char *)malloc(strlen(value) + 1); // +1 for null terminator
		if (node->attributes[i-2].value == NULL) {
			perror("Failed to allocate memory for value\n");
			exit(1);
		}

		// Now that memory is allocated, copy the strings
		strcpy(node->attributes[i-2].attr_name, key);
		strcpy(node->attributes[i-2].value, value);
	}

	// Start logging the node
	node->log_count = 1;
	node->oldest_log_index = 0;
	node->log[node->log_count++] = time(NULL);

	// fill local registry
	local_reg.nodes[local_reg.num_nodes++] = *node;

	return 0;
}

// @Description : Initialize the app listener thread
// will send one discovery message at startup and listen for
// notifications and heartbeats throughout the lifetime of the app
void* init_app(void* arg) {
    // First send a discovery to the multicast group
    // need to encode who is sending the notification
    char discovery_msg[BUF_SIZE];
	char *received_data[MAX_MSG_SIZE];
	int dsize;
	char *token;

	strcpy(discovery_msg, "msgType:DISCOVERY;");

    char discovery_buffer[BUF_SIZE];

	multicast_send(zcs_node.msend, discovery_msg, strlen(discovery_msg)+1);

	// wait for incoming messages NOTIFICATION messages
	while(1) {
		if (multicast_check_receive(zcs_node.mrecv) > 0) {
			multicast_receive(zcs_node.mrecv, discovery_buffer, BUF_SIZE);
			// print the received message
			if (strstr(discovery_buffer, "msgType:NOTIFICATION;") != NULL) {
				printf("Notification received: %s\n", discovery_buffer);
				// if we receive a notification, check if we have already received it
				
				// @FIXME
				if (find_node(&local_reg, zcs_node.name) != -1) {
					// we have already received the notification
					continue;
				}
				token = strtok(discovery_buffer, ";");
				dsize = 0;
				while (token != NULL) {
					received_data[dsize] = (char*) malloc((strlen(token) + 1) * sizeof(char));
					strcpy(received_data[dsize++], token);
					token = strtok(NULL, ";");
				}

				// make a registry entry
				make_reg_entry(received_data, dsize);

				for (int i = 0; i < dsize; i++) {
					free(received_data[i]);
					received_data[i] = NULL;
				}
			} else if (strstr(discovery_buffer, "msgType:HEARTBEAT;") != NULL) {
				// the message is a heartbeat, we need to parse it
				printf("Heartbeat received: %s\n", discovery_buffer);
				// First throw away msgType:HEARTBEAT;
				char *str;
				str = strtok(discovery_buffer, ";");
				str = strtok(NULL, ";");

				// now we have nodeName:node_name

				char *key, *value;
				split_key_value(str, &key, &value);
				// check if the node is in the local registry
				// value is returning everything before the value, why?

				int index = find_node(&local_reg, value);
				
				if (index != -1) {
					// the node is in the local registry, update the log
					add_log(&local_reg.nodes[index]);
				}
			}
		}
	}
	return 0;
}

// @Description : Send a notification at service startup and on discovery message
void* notification(void* arg) {
	// FORMAT: "msgType:NOTIFICATION;nodeName:node_name;attr1:val1;attr2:val2;attr3:val3"
	char msg[BUF_SIZE];
	// buffer to hold received messages
	char buffer[BUF_SIZE];
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
	multicast_send(zcs_node.msend, msg, strlen(msg)+1);

	// wait for incoming messages DISCOVERY messages
	while(1){
		// check for incoming messages
		if (multicast_check_receive(zcs_node.mrecv) > 0) {
			// print the check receive value
			multicast_receive(zcs_node.mrecv, buffer, BUF_SIZE);
			if (strstr(buffer, "msgType:DISCOVERY;") != NULL) {
				printf("Sending notification: %s\n", msg);
				zcs_multicast_send(msg);
			}
		}
	}
	return NULL;
}

// @Description : Send a heartbeat message to the multicast group
void* heartbeat(void* arg) {
	char heartbeat_msg[BUF_SIZE];
	char msg[BUF_SIZE];
	strcpy(heartbeat_msg, "msgType:HEARTBEAT;nodeName:");
	strcat(heartbeat_msg, zcs_node.name);
	strcat(heartbeat_msg, ";");
	strcat(heartbeat_msg, "\0");

    while(1) {
		// not expecting any incoming messages, just send the heartbeat
		printf("sending heartbeat...\n");
		multicast_send(zcs_node.msend, heartbeat_msg, strlen(heartbeat_msg)+1);
		sleep(HEARTBEAT_INTERVAL);
    }
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

    // @TODO : should init_app be a thread? We want the app to keep listening and 
	// logging the messages it receives...
	if (type == ZCS_APP_TYPE) {
		// @TODO : thead
		appThread = (pthread_t*) malloc(sizeof(pthread_t));
		if (!appThread) {
			perror("zcs_init: bad malloc\n");
			return -1;
		}
		if (pthread_create(appThread, NULL, &init_app, NULL)) {
			perror("zcs_init: bad pthread_create\n");
			return -1;
		}
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
	// should we do -2 to account for the nodeName and msgType?
	if (num > MAX_MSG_SIZE - 2) {
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

	if (zcs_node.type == ZCS_SERVICE_TYPE) {
		// create a listener thread that will listen for incoming DISCOVERY messages
		// and send a notification
		notificationThread = (pthread_t*) malloc(sizeof(pthread_t));
		if (pthread_create(notificationThread, NULL, &notification, NULL) || !notificationThread) {
			perror("zcs_start: pthread_create\n");
			return -1;
		}

		// Start the service status thread which will send heartbeats
		// and listen for incoming messages
		heartbeatThread = (pthread_t*) malloc(sizeof(pthread_t));

		if (pthread_create(heartbeatThread, NULL, &heartbeat, NULL) || !heartbeatThread) {
			perror("zcs_start: pthread_create\n");
			return -1;
		}
	}

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
		if (multicast_check_receive(zcs_node.mrecv) > 0) {
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
    printf("---- Node Status Logs ----\n");
    for (int i = 0; i < local_reg.num_nodes; i++) {
        zcs_node_t *node = &local_reg.nodes[i];
        printf("Logs for %s:\n", node->name);
        
        if (node->log_count == 0) {
            printf("No logs available.\n");
            continue;
        }

        int current_index = node->oldest_log_index;
        time_t current_time = node->log[current_index];
        time_t next_time;
        int isUp = 1; // Assuming the node starts in UP state
        time_t upStartTime = current_time;
        time_t downStartTime;

        for (int j = 0; j < node->log_count; j++) {
            int next_index = (current_index + 1) % LOG_SIZE;
            next_time = node->log[next_index];

            // Check if we have wrapped around
            if (next_index == node->oldest_log_index) break;

            double timeDiff = difftime(next_time, current_time);
            if (isUp && timeDiff > HEARTBEAT_INTERVAL + 1) {
                // Transition from UP to DOWN
                printf("UP: %s -> %s\n", ctime(&upStartTime), ctime(&current_time));
                downStartTime = current_time;
                isUp = 0;
            } else if (!isUp && timeDiff <= HEARTBEAT_INTERVAL + 1) {
                // Transition from DOWN to UP
                printf("DOWN: %s -> %s\n", ctime(&downStartTime), ctime(&current_time));
                upStartTime = current_time;
                isUp = 1;
            }

            current_index = next_index;
            current_time = next_time;
        }


        // Handle the last sequence
        if (isUp) {
            printf("UP: %s -> now\n", ctime(&upStartTime));
        } else {
            printf("DOWN: %s -> now\n", ctime(&downStartTime));
        }
    }
    printf("-------------------------\n");
}

int zcs_shutdown() {
	// free the memory allocated for the threads and join them
	// only services will have threads
	if (zcs_node.type == ZCS_SERVICE_TYPE) {
		pthread_join(*heartbeatThread, NULL);
		pthread_cancel(*heartbeatThread);
		free(heartbeatThread);
		pthread_join(*notificationThread, NULL);
		pthread_cancel(*notificationThread);
		free(notificationThread);
		free_node_attributes(&zcs_node);
	} else if (zcs_node.type == ZCS_APP_TYPE) {
		pthread_join(*appThread, NULL);
		pthread_cancel(*appThread);
		free(appThread);
		// print the local registry
		print_local_reg(&local_reg);
		// free the memory allocated for the local registry
		for (int i = 0; i < local_reg.num_nodes; i++) {
        	free_node_attributes(&local_reg.nodes[i]);
		}
	}
    // free the memory allocated for the multicast object
    multicast_destroy(zcs_node.msend);
	multicast_destroy(zcs_node.mrecv);

    return 0;
}
