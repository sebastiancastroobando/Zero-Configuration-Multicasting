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

// node object
typedef struct {
	char name[MAX_NAME_LEN];
    int type;
    zcs_attribute_t *attributes;
    mcast_t *msend;
	mcast_t *mrecv;
	mcast_t *m_ad_send;
	mcast_t *m_ad_recv;
    int num_attributes;
	time_t log[LOG_SIZE];
    int oldest_log_index;
    int log_count;
} zcs_node_t;

// local registry object
typedef struct {
	zcs_node_t *nodes[MAX_SIZE];
	int num_nodes;
} zcs_reg_t;

// thread arguments object
typedef struct {
	char *name;
	zcs_cb_f cback;
} zcs_thread_args;

// All of the memory the current library instance shares between threads
static zcs_node_t zcs_node;
pthread_t *heartbeatThread;
pthread_t *notificationThread;
pthread_t *appThread;
pthread_t *listenAdThread;
zcs_thread_args *ad_args;
zcs_reg_t local_reg;
pthread_mutex_t msend_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for msend and mrecv
volatile int listenToAd = 0; // listenToAd flag to let the app know that it should listen for ads


// To ensure graceful library termination, used to signal threads to stop
// volatile to ensure that the compiler does not optimize it away
volatile int keep_running = 1; 

// ------------------- Helper functions (Debugging) -------------------
/**
 * @brief Print the node object
*/
void print_node(zcs_node_t *node) {
	printf("name: %s\n", node->name);
	printf("type: %d\n", node->type);
	printf("num_attributes: %d\n", node->num_attributes);
	for (int i = 0; i < node->num_attributes; i++) {
		printf("attr_name: %s\n", node->attributes[i].attr_name);
		printf("value: %s\n", node->attributes[i].value);
	}
}
/**
 * @brief Print the local registry
*/
void print_local_reg(zcs_reg_t *reg) {
	for (int i = 0; i < reg->num_nodes; i++) {
		print_node(reg->nodes[i]);
	}
}
// ---------------------------------------------------------------------

// --------------- Helper functions (Memory Management) ----------------
/**
 * @brief Free the memory allocated for the attributes of a node
*/
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
// ---------------------------------------------------------------------

/**
 * @brief Add a log entry to the node
*/
void add_log(zcs_node_t *node) {
	if (node->log_count >= LOG_SIZE) {
		// we have reached the max number of log entries
		// we need to remove the oldest log entry
		node->log[node->oldest_log_index] = time(NULL);
		node->oldest_log_index = (node->oldest_log_index + 1) % LOG_SIZE;
	} else {
		node->log[node->log_count++] = time(NULL);
	}
}

/**
 * @brief Find the index of a node in the local registry
 * @return the index of the node in the local registry, -1 if not found
*/
int find_node(char *name) {
	for (int i = 0; i < local_reg.num_nodes; i++) {
		if (memcmp(local_reg.nodes[i]->name, name, sizeof(name) + sizeof(char)) == 0)
			return i;
	}
	return -1;
}

/**
 * @brief Split a string into key and value
*/
void split_key_value(char *str, char **key, char **value) {
	// Expected Format : "key:value"
	*key = strtok(str, ":");
	*value = strtok(NULL, ":");
}

/**
 * @brief Make a registry entry for a node
 * @return 0 on success, 1 on failure
*/
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
	node->log_count = 0;
	node->oldest_log_index = 0;

	// fill local registry
	local_reg.nodes[local_reg.num_nodes++] = node;

	return 0;
}

/**
 * @brief Initialize the app listener thread
 * @details The app listener thread will send one discovery message at startup and listen for notifications and heartbeats throughout the lifetime of the app
 * @return 0 on success, -1 on failure
*/
void* init_app(void* arg) {
    // First send a discovery to the multicast group
    // need to encode who is sending the notification
    char discovery_msg[BUF_SIZE];
	char *received_data[MAX_MSG_SIZE];
	int dsize;
	char *token, *key, *value;

	strcpy(discovery_msg, "msgType:DISCOVERY;");

    char discovery_buffer[BUF_SIZE];

	multicast_send(zcs_node.msend, discovery_msg, strlen(discovery_msg)+1);

	// Poll for incoming messages
	while(keep_running) {
		if (multicast_check_receive(zcs_node.mrecv) > 0) {
			multicast_receive(zcs_node.mrecv, discovery_buffer, BUF_SIZE);
			// tokenize by semi-colon
			token = strtok(discovery_buffer, ";");
			dsize = 0;
			while (token != NULL) {
				received_data[dsize] = (char*) malloc((strlen(token) + 1) * sizeof(char));
				strcpy(received_data[dsize++], token);
				token = strtok(NULL, ";");
			}

			if (memcmp(received_data[0], "msgType:NOTIFICATION", sizeof("msgType:NOTIFICATION") + sizeof(char)) == 0) {
				// FORMAT: "msgType:NOTIFICATION;nodeName:node_name;attr1:val1;attr2:val2;attr3:val3..."
				// if we receive a notification, check if we have already received it
				char *copy = (char*) malloc(strlen(received_data[1]) + 1);
				strcpy(copy, received_data[1]);
				split_key_value(copy, &key, &value);
				if (find_node(value) != -1) {
					// we have already received the notification, ignore it
					// free allocated data
					free(copy);
					for (int i = 0; i < dsize; i++) {
						free(received_data[i]);
						received_data[i] = NULL;
					}
					continue;
				}
				free(copy);
				// make a registry entry
				make_reg_entry(received_data, dsize);
			} 
			else if (memcmp(received_data[0], "msgType:HEARTBEAT", sizeof("msgType:HEARTBEAT") + sizeof(char)) == 0) {
				// FORMAT: "msgType:HEARTBEAT;nodeName:node_name"
				// the message is a heartbeat, we need to parse it
				split_key_value(received_data[1], &key, &value);
				// check if the node is in the local registry
				int index = find_node(value);
				if (index != -1) {
					// the node is in the local registry, update the log
					add_log(local_reg.nodes[index]);
				}
			}
			else if (memcmp(received_data[0], "msgType:AD", sizeof("msgType:AD") + sizeof(char)) == 0 && listenToAd == 1) {
				// FORMAT: "msgType:AD;nodeName:node_name;adName:ad_name;adValue:ad_value"
				// the message is an ad, we need to check if nodeName matches the name we are listening to
				// if not, ignore the message
				split_key_value(received_data[1], &key, &value); // key = nodeName, value = node_name
				if (memcmp(ad_args->name, value, sizeof(value) + sizeof(char)) == 0) {
					split_key_value(received_data[2], &key, &value); // key = adName, value = ad_name
					split_key_value(received_data[3], &key, &value); // key = adValue, value = ad_value
					// call the callback function
					ad_args->cback(received_data[2], received_data[3]); 
				}
			}

			// free receive data buffer
			for (int i = 0; i < dsize; i++) {
				free(received_data[i]);
				received_data[i] = NULL;
			}
		}
	}
	return NULL;
}

/**
 * @brief Send a notification to the multicast group
 * @details This function will be called by the notification thread, which will send a notification at startup and on receiving a discovery message
*/
void* notification(void* arg) {
	// FORMAT: "msgType:NOTIFICATION;nodeName:node_name;attr1:val1;attr2:val2;attr3:val3"
	char notif_msg[BUF_SIZE];
	// buffer to hold received messages
	char buffer[BUF_SIZE];
	strcpy(notif_msg, "msgType:NOTIFICATION;");
	strcat(notif_msg, "nodeName:");
	strcat(notif_msg, zcs_node.name);
	strcat(notif_msg, ";");
	for (int i = 0; i < zcs_node.num_attributes; i++) {
		strcat(notif_msg, zcs_node.attributes[i].attr_name);
		strcat(notif_msg, ":");
		strcat(notif_msg, zcs_node.attributes[i].value);
		strcat(notif_msg, ";");
	}

	// send the notification to the multicast group
	multicast_send(zcs_node.msend, notif_msg, strlen(notif_msg) + 1);

	// wait for incoming messages DISCOVERY messages
	while(keep_running){
		// check for incoming messages
		if (multicast_check_receive(zcs_node.mrecv) > 0) {
			// print the check receive value
			multicast_receive(zcs_node.mrecv, buffer, BUF_SIZE);
			if (strstr(buffer, "msgType:DISCOVERY;") != NULL) {
				printf("sending notification...\n");
				// send the notification to the multicast group
				// lock the msend mutex to ensure that it does not interfere with another thread
				pthread_mutex_lock(&msend_mutex);
				multicast_send(zcs_node.msend, notif_msg, strlen(notif_msg)+1);
				pthread_mutex_unlock(&msend_mutex);
			}
		}
	}
	return NULL;
}

/**
 * @brief Send a heartbeat to the multicast group
 * @details This function will be called by the heartbeat thread, which will send a heartbeat at regular intervals
*/
void* heartbeat(void* arg) {
	char heartbeat_msg[BUF_SIZE];
	strcpy(heartbeat_msg, "msgType:HEARTBEAT;nodeName:");
	strcat(heartbeat_msg, zcs_node.name);
	strcat(heartbeat_msg, ";");
	strcat(heartbeat_msg, "\0");

	sleep(1);
    while(keep_running) {
		// not expecting any incoming messages, just send the heartbeat
		printf("sending heartbeat...\n");
		// send the heartbeat to the multicast group
		// lock the msend mutex to ensure that it does not interfere with another thread
		pthread_mutex_lock(&msend_mutex);
		multicast_send(zcs_node.msend, heartbeat_msg, strlen(heartbeat_msg)+1);
		pthread_mutex_unlock(&msend_mutex);
		sleep(HEARTBEAT_INTERVAL);
    }
	return NULL;
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
	// specify that there are no nodes in local registry
	local_reg.num_nodes = 0;
	
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

	// save the multicast object to the node object for
	// notifications, heartbeats, and discovery
	zcs_node.type = type;
    zcs_node.msend = msend;
	zcs_node.mrecv = mrecv;

	// logging the messages it receives...
	if (type == ZCS_APP_TYPE) {
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
 * @brief Puts a service node online
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
		// send notification
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
 * @brief Post an ad to the ad multicast group
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

	// Send the ad to the multicast group at ad_rate intervals
	while (attempts < MAX_AD_ATTEMPTS) {
		// FORMAT: "msgType:AD;nodeName:node_name;adName:ad_name;adValue:ad_value" ? 
		char ad_msg[BUF_SIZE];
		strcpy(ad_msg, "msgType:AD;nodeName:");
		strcat(ad_msg, zcs_node.name);
		strcat(ad_msg, ";adName:");
		strcat(ad_msg, ad_name);
		strcat(ad_msg, ";adValue:");
		strcat(ad_msg, ad_value);
		strcat(ad_msg, ";");

		// send the ad to the multicast group, but first lock the msend mutex
		// this is to ensure that it does interfere with another thread
		pthread_mutex_lock(&msend_mutex);
		multicast_send(zcs_node.msend, ad_msg, strlen(ad_msg)+1);
		pthread_mutex_unlock(&msend_mutex);
		// multicast_send(zcs_node.m_ad_send, ad_msg, strlen(ad_msg)+1);
		attempts++;
		sleep(ad_rate);
	}

    return attempts;
}

/**
 * @brief Signals the app to listen for ads with a given name
 * @return 0 on success, -1 on failure
*/
int zcs_listen_ad(char *name, zcs_cb_f cback) {
	int ret = -1;
	// create a thread that will listen for ads
	ad_args = (zcs_thread_args*) malloc(sizeof(zcs_thread_args));

	// check if the node is an app
	if (zcs_node.type == ZCS_APP_TYPE) {
		ad_args->name = name;
		ad_args->cback = cback;
		// set the listenToAd flag to 1, this will allow the app to listen for ads
		listenToAd = 1;
	} else {
		perror("zcs_listen_ad: node is not an app\n");
	}
	// free(ad_args); we should not free ad_args, it is used by the thread
	// free it in the shutdown function
	return ret;
}

/**
 * @brief Scan for nodes with a given value for a given attribute.
 * @details This function will scan the local registry for nodes that match the query
 * @return the number of nodes that match the query
*/
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen) {
	// check if there is something to query
	if (local_reg.num_nodes == 0) {
		// sleep for a second to allow the app to receive notifications
		sleep(1);
	}
	int cnt = 0;
	if (local_reg.num_nodes < namelen)
		namelen = local_reg.num_nodes;
	for (int i = 0; i < namelen; i++) {
		if (strcmp(local_reg.nodes[i]->attributes->attr_name, attr_name) == 0
				&& strcmp(local_reg.nodes[i]->attributes->value, attr_value) == 0) {
			node_names[cnt++] = local_reg.nodes[i]->name;
		}
	}
	return cnt;
}

/**
 * @brief get full list of attributes of a node that is return by a query
 * @return 0 on success, -1 on failure
*/
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num) {
	// check if the node is in the local registry
	int index = find_node(name);
	int ret = -1;
	if (index != -1) {
		if (local_reg.nodes[index]->num_attributes < *num)
			*num = local_reg.nodes[index]->num_attributes;
		memcpy(attr, local_reg.nodes[index]->attributes, *num * sizeof(zcs_attribute_t));
		ret = 0;
	}
	return ret;
}

/**
 * @brief Log the message to the console
*/
void zcs_log() {
    printf("---- Node Status Logs ----\n");
    for (int i = 0; i < local_reg.num_nodes; i++) {
        zcs_node_t node = *local_reg.nodes[i];
        printf("Logs for %s:\n", node.name);
        
        if (node.log_count == 0) {
            printf("No logs available.\n");
            continue;
        }

		char start_time_str[9], end_time_str[9]; // Buffer to hold "HH:MM:SS\0"
        int start_index = node.oldest_log_index;
		int next_index;
        time_t current_time = node.log[start_index];
        time_t next_time;
		double timeDiff;

		for (int j = 1; j < node.log_count; j++) {
            next_index = (start_index + j) % LOG_SIZE;
			next_time = node.log[next_index];

			strftime(start_time_str, sizeof(start_time_str), "%H:%M:%S", localtime(&current_time));
			strftime(end_time_str, sizeof(end_time_str), "%H:%M:%S", localtime(&next_time));
			
            timeDiff = difftime(next_time, current_time);
            if (timeDiff > HEARTBEAT_INTERVAL + 1) {
                // Transition from UP to DOWN
                printf("DOWN : %s -> %s\n", start_time_str, end_time_str);
            } else {
                // Transition from DOWN to UP
                printf("UP   : %s -> %s\n", start_time_str, end_time_str);
            }

            current_time = next_time;
        }

        // Handle the last sequence
		int now_time = time(NULL);
		timeDiff = difftime(now_time, current_time);

		// format the current time to HH:MM:SS
		char current_time_str[9];
		strftime(current_time_str, sizeof(start_time_str), "%H:%M:%S", localtime(&current_time));

        if (timeDiff > HEARTBEAT_INTERVAL + 1) {
            printf("DOWN : %s -> now\n", current_time_str);
        } else {
            printf("UP   : %s -> now\n", current_time_str);
		}
    }
    printf("-------------------------\n");
}

/**
 * @brief Shutdown the node
 * @return 0 on success, -1 on failure
 * @todo we are not doing any error checking here...
*/
int zcs_shutdown() {
	// free the memory allocated for the threads and join them
	// only services will have threads

	keep_running = 0;

	if (zcs_node.type == ZCS_SERVICE_TYPE) {
		pthread_join(*heartbeatThread, NULL);
		pthread_cancel(*heartbeatThread);
		free(heartbeatThread);
		pthread_join(*notificationThread, NULL);
		pthread_cancel(*notificationThread);
		free(notificationThread);
		free(zcs_node.attributes);
	} else if (zcs_node.type == ZCS_APP_TYPE) {
		pthread_join(*appThread, NULL);
		free(appThread);
		if (listenAdThread != NULL) {
			pthread_join(*listenAdThread, NULL);
			free(listenAdThread);
		}
		// free the memory allocated for the local registry
		for (int i = 0; i < local_reg.num_nodes; i++) {
			free_node_attributes(local_reg.nodes[i]);
			free(local_reg.nodes[i]);
		}
		// check if ad_args is not NULL
		if (ad_args != NULL) {
			free(ad_args);
		}
	}
    // free the memory allocated for the multicast object
    multicast_destroy(zcs_node.msend);
	multicast_destroy(zcs_node.mrecv);
	multicast_destroy(zcs_node.m_ad_send);
	multicast_destroy(zcs_node.m_ad_recv);

    return 0;
}
