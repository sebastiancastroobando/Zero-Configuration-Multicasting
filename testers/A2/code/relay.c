#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "multicast.h"
#include "relay.h"

#define MAX_MSG_SIZE 1000       // maximum size of the multicast message to be relayed
#define VERBOSE 1               // 1 for verbose, 0 for non-verbose

// keep running the relay
static int keep_running = 1;

mcast_t *LAN_A_CHANNEL1_mrecv; // multicast receiver Channel 1 for LAN_A
mcast_t *LAN_A_CHANNEL1_msend; // multicast sender Channel 1 for LAN_A
mcast_t *LAN_A_CHANNEL2_mrecv; // multicast receiver Channel 2 for LAN_A
mcast_t *LAN_A_CHANNEL2_msend; // multicast sender Channel 2 for LAN_A
mcast_t *LAN_B_CHANNEL1_mrecv; // multicast receiver Channel 1 for LAN_B
mcast_t *LAN_B_CHANNEL1_msend; // multicast sender Channel 1 for LAN_B
mcast_t *LAN_B_CHANNEL2_mrecv; // multicast receiver Channel 2 for LAN_B
mcast_t *LAN_B_CHANNEL2_msend; // multicast sender Channel 2 for LAN_B

// We need four threads to handle the multicast the relay between corresponding channels in LAN_A and LAN_B
pthread_t *relay1_thread;
pthread_t *relay2_thread;
pthread_t *relay3_thread;
pthread_t *relay4_thread;

// define struct to hold mrecv, msend, and general info
typedef struct {
    char* name;
    mcast_t *mrecv;
    mcast_t *msend;
    char *channel_source;
    char *channel_destination;
} relay_info_t;

relay_info_t *relay1;
relay_info_t *relay3;
relay_info_t *relay2;
relay_info_t *relay4;

/**
 * @brief function to be executed by the thread that relays multicast messages between LAN_A and LAN_B
 * @param mrecv multicast receiver object from first LAN
 * @param msend multicast sender object from second LAN
*/
void *relay_thread(void *arg) {
    // The relay thread will take one receiver from a channel and send it to the other channel with a sender.
    // get the relay info
    relay_info_t *relay_info = (relay_info_t *)arg;

    char received_data[MAX_MSG_SIZE];

    // receive the multicast message
    while(keep_running) {
        if (multicast_check_receive(relay_info->mrecv) > 0) {
            multicast_receive(relay_info->mrecv, received_data, MAX_MSG_SIZE); // receive the multicast message

            // check if the received message is already relayed
            if (strstr(received_data, "relayed:true") != NULL) {
                continue; // if the message is already relayed, then skip it
            }

            // First create the message to be concatenated with the received message
            // ...;relayed:true;transmission:LAN_A_CHANNEL1->LAN_B_CHANNEL1;
            char *transmission = (char *)malloc(sizeof(char) * 100);
            strcpy(transmission, "relayed:true;transmission:");
            strcat(transmission, relay_info->channel_source);
            strcat(transmission, "->");
            strcat(transmission, relay_info->channel_destination);
            strcat(transmission, ";");

            // concatenate the received message with the transmission message
            // @todo: check if the received message is too large? 
            strcat(received_data, transmission);
            if (VERBOSE) {
                printf("RELAYING\t%s\n", received_data);
            }
            
            // send the multicast message, +1 to include the null terminator
            multicast_send(relay_info->msend, received_data, strlen(received_data)+1);

            // free the memory allocated for the transmission message
            free(transmission);
        }
    }
}

/**
 * @brief Create struct to hold the multicast receiver and sender objects
*/
void create_relay_info(relay_info_t **relay_info, char* name, mcast_t *mrecv, mcast_t *msend, char *channel_source, char *channel_destination) {
    *relay_info = (relay_info_t *)malloc(sizeof(relay_info_t));
    (*relay_info)->name = name;
    (*relay_info)->mrecv = mrecv;
    (*relay_info)->msend = msend;
    (*relay_info)->channel_source = channel_source;
    (*relay_info)->channel_destination = channel_destination;
}

/**
 * @brief Initialize the multicast groups for LAN_A and LAN_B
*/
void relay_init(char *channel1_LAN_A, char *channel2_LAN_A, int LAN_A_port1, int LAN_A_port2, char *channel1_LAN_B, char *channel2_LAN_B, int LAN_B_port1, int LAN_B_port2) {
    /**
     * Ports are mirrored between channels in a given LAN, meaning :
     * for an APP in LAN_A, it would be :
     * - msend : {channel1, port, port+1}; mrecv: {channel2, port-1, port}. 
     * and for a SERVICE in LAN_A, it would be:
     * - msend : {channel2, port, port+1}; mrecv: {channel1, port-1, port}.
    */
    
    // initialize the multicast groups for LAN_A
    LAN_A_CHANNEL1_mrecv = multicast_init(channel1_LAN_A, LAN_A_port1-1, LAN_A_port1); // relay #1 recv
    LAN_A_CHANNEL1_msend = multicast_init(channel1_LAN_A, LAN_A_port1, LAN_A_port1+1); // relay #2 send
    LAN_A_CHANNEL2_mrecv = multicast_init(channel2_LAN_A, LAN_A_port2-1, LAN_A_port2); // relay #3 recv
    LAN_A_CHANNEL2_msend = multicast_init(channel2_LAN_A, LAN_A_port2, LAN_A_port2+1); // relay #4 send

    // initialize the multicast groups for LAN_B
    LAN_B_CHANNEL1_mrecv = multicast_init(channel1_LAN_B, LAN_B_port1-1, LAN_B_port1); // relay #2 recv
    LAN_B_CHANNEL1_msend = multicast_init(channel1_LAN_B, LAN_B_port1, LAN_B_port1+1); // relay #1 send
    LAN_B_CHANNEL2_mrecv = multicast_init(channel2_LAN_B, LAN_B_port2-1, LAN_B_port2); // relay #4 recv
    LAN_B_CHANNEL2_msend = multicast_init(channel2_LAN_B, LAN_B_port2, LAN_B_port2+1); // relay #3 send

    // setup receiver multicast objects
    multicast_setup_recv(LAN_A_CHANNEL1_mrecv);
    multicast_setup_recv(LAN_A_CHANNEL2_mrecv);
    multicast_setup_recv(LAN_B_CHANNEL1_mrecv);
    multicast_setup_recv(LAN_B_CHANNEL2_mrecv);

    // create the threads for the relay
    relay1_thread = (pthread_t *)malloc(sizeof(pthread_t));
    relay2_thread = (pthread_t *)malloc(sizeof(pthread_t));
    relay3_thread = (pthread_t *)malloc(sizeof(pthread_t));
    relay4_thread = (pthread_t *)malloc(sizeof(pthread_t));

    // create the relay info structs and assign them to the global variables
    // FORMAT: relay_info ; mrecv ; msend ; channel_source ; channel_destination
    create_relay_info(&relay1, "relay1", LAN_A_CHANNEL1_mrecv, LAN_B_CHANNEL1_msend, channel1_LAN_A, channel1_LAN_B); // relay #1
    create_relay_info(&relay3, "relay3", LAN_A_CHANNEL2_mrecv, LAN_B_CHANNEL2_msend, channel2_LAN_A, channel2_LAN_B); // relay #3
    create_relay_info(&relay2, "relay2", LAN_B_CHANNEL1_mrecv, LAN_A_CHANNEL1_msend, channel1_LAN_B, channel1_LAN_A); // relay #2
    create_relay_info(&relay4, "relay4", LAN_B_CHANNEL2_mrecv, LAN_A_CHANNEL2_msend, channel2_LAN_B, channel2_LAN_A); // relay #4
    
    // start the relay threads
    pthread_create(relay1_thread, NULL, &relay_thread, (void *)relay1);
    pthread_create(relay2_thread, NULL, &relay_thread, (void *)relay2);
    pthread_create(relay3_thread, NULL, &relay_thread, (void *)relay3);
    pthread_create(relay4_thread, NULL, &relay_thread, (void *)relay4);
}

/**
 * @brief Shutdown the relay
*/
void shutdown_relay() {
    // stop the relay
    keep_running = 0;

    // wait for the threads to finish
    pthread_join(*relay1_thread, NULL);
    pthread_join(*relay2_thread, NULL);
    pthread_join(*relay3_thread, NULL);
    pthread_join(*relay4_thread, NULL);

    // free the memory allocated for the threads
    free(relay1_thread);
    free(relay2_thread);
    free(relay3_thread);
    free(relay4_thread);

    // free the memory allocated for the relay info structs
    free(relay1);
    free(relay3);
    free(relay2);
    free(relay4);

    // destroy the multicast groups
    multicast_destroy(LAN_A_CHANNEL1_mrecv);
    multicast_destroy(LAN_A_CHANNEL1_msend);
    multicast_destroy(LAN_A_CHANNEL2_mrecv);
    multicast_destroy(LAN_A_CHANNEL2_msend);

    multicast_destroy(LAN_B_CHANNEL1_mrecv);
    multicast_destroy(LAN_B_CHANNEL1_msend);
    multicast_destroy(LAN_B_CHANNEL2_mrecv);
    multicast_destroy(LAN_B_CHANNEL2_msend);
}

/**
 * @brief Relay the multicast messages between LAN_A and LAN_B
 * @todo should this be implemented like a library instead of a main function?
*/
int main() {
    // initialize the multicast groups for LAN_A and LAN_B
    relay_init(LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT1, LAN_A_PORT2, LAN_B_CHANNEL1, LAN_B_CHANNEL2, LAN_B_PORT1, LAN_B_PORT2);
    sleep(30); // sleep for 30 seconds
    shutdown_relay(); // shutdown the relay

    return 0;
}
