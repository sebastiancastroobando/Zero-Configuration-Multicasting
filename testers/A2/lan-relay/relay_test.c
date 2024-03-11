#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "../../../multicast.h"

#define MAX_MSG_SIZE 1000       // maximum size of the multicast message to be relayed
#define VERBOSE 1               // 1 for verbose, 0 for non-verbose

// keep running the relay
static int keep_running = 1;

mcast_t *mcast_LAN1_CHANNEL1_mrecv; // multicast receiver Channel 1 for LAN1
mcast_t *mcast_LAN1_CHANNEL1_msend; // multicast sender Channel 1 for LAN1
mcast_t *mcast_LAN1_CHANNEL2_mrecv; // multicast receiver Channel 2 for LAN1
mcast_t *mcast_LAN1_CHANNEL2_msend; // multicast sender Channel 2 for LAN1
mcast_t *mcast_LAN2_CHANNEL1_mrecv; // multicast receiver Channel 1 for LAN2
mcast_t *mcast_LAN2_CHANNEL1_msend; // multicast sender Channel 1 for LAN2
mcast_t *mcast_LAN2_CHANNEL2_mrecv; // multicast receiver Channel 2 for LAN2
mcast_t *mcast_LAN2_CHANNEL2_msend; // multicast sender Channel 2 for LAN2

// We need four threads to handle the multicast the relay between corresponding channels in LAN1 and LAN2
pthread_t *LAN1_CHANNEL1_thread;
pthread_t *LAN1_CHANNEL2_thread;
pthread_t *LAN2_CHANNEL1_thread;
pthread_t *LAN2_CHANNEL2_thread;

// define struct to hold mrecv, msend, and general info
typedef struct {
    mcast_t *mrecv;
    mcast_t *msend;
    char *channel_source;
    char *channel_destination;
} relay_info_t;

relay_info_t *relay_info_LAN1_CHANNEL1;
relay_info_t *relay_info_LAN1_CHANNEL2;
relay_info_t *relay_info_LAN2_CHANNEL1;
relay_info_t *relay_info_LAN2_CHANNEL2;

/**
 * @brief function to be executed by the thread that relays multicast messages between LAN1 and LAN2
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
            // ...;relayed:true;transmission:LAN1_CHANNEL1->LAN2_CHANNEL1;
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
void create_relay_info(relay_info_t **relay_info, mcast_t *mrecv, mcast_t *msend, char *channel_source, char *channel_destination) {
    *relay_info = (relay_info_t *)malloc(sizeof(relay_info_t));
    (*relay_info)->mrecv = mrecv;
    (*relay_info)->msend = msend;
    (*relay_info)->channel_source = channel_source;
    (*relay_info)->channel_destination = channel_destination;
}

/**
 * @brief Initialize the multicast groups for LAN1 and LAN2
*/
void relay_init(char *channel1_LAN1, char *channel2_LAN1, int port_LAN1, char *channel1_LAN2, char *channel2_LAN2, int port2_LAN2) {
    /**
     * Ports are mirrored between channels in a given LAN, meaning :
     * for an APP in LAN1, it would be :
     * - msend : {channel1, port, port+1}; mrecv: {channel2, port-1, port}. 
     * and for a SERVICE in LAN1, it would be:
     * - msend : {channel2, port, port+1}; mrecv: {channel1, port-1, port}.
    */
    
    // initialize the multicast groups for LAN1
    mcast_LAN1_CHANNEL1_mrecv = multicast_init(channel1_LAN1, port_LAN1-1, port_LAN1);
    mcast_LAN1_CHANNEL1_msend = multicast_init(channel1_LAN1, port_LAN1, port_LAN1+1);
    mcast_LAN1_CHANNEL2_mrecv = multicast_init(channel2_LAN1, port_LAN1-1, port_LAN1);
    mcast_LAN1_CHANNEL2_msend = multicast_init(channel2_LAN1, port_LAN1, port_LAN1+1);

    // initialize the multicast groups for LAN2
    mcast_LAN2_CHANNEL1_mrecv = multicast_init(channel1_LAN2, port2_LAN2-1, port2_LAN2);
    mcast_LAN2_CHANNEL1_msend = multicast_init(channel1_LAN2, port2_LAN2, port2_LAN2+1);
    mcast_LAN2_CHANNEL2_mrecv = multicast_init(channel2_LAN2, port2_LAN2-1, port2_LAN2);
    mcast_LAN2_CHANNEL2_msend = multicast_init(channel2_LAN2, port2_LAN2, port2_LAN2+1);

    // setup receiver multicast objects
    multicast_setup_recv(mcast_LAN1_CHANNEL1_mrecv);
    multicast_setup_recv(mcast_LAN1_CHANNEL2_mrecv);
    multicast_setup_recv(mcast_LAN2_CHANNEL1_mrecv);
    multicast_setup_recv(mcast_LAN2_CHANNEL2_mrecv);

    // create the threads for the relay
    LAN1_CHANNEL1_thread = (pthread_t *)malloc(sizeof(pthread_t));
    LAN1_CHANNEL2_thread = (pthread_t *)malloc(sizeof(pthread_t));
    LAN2_CHANNEL1_thread = (pthread_t *)malloc(sizeof(pthread_t));
    LAN2_CHANNEL2_thread = (pthread_t *)malloc(sizeof(pthread_t));

    // create the relay info structs and assign them to the global variables
    create_relay_info(&relay_info_LAN1_CHANNEL1, mcast_LAN1_CHANNEL1_mrecv, mcast_LAN2_CHANNEL1_msend, channel1_LAN1, channel1_LAN2);
    create_relay_info(&relay_info_LAN1_CHANNEL2, mcast_LAN1_CHANNEL2_mrecv, mcast_LAN2_CHANNEL2_msend, channel2_LAN1, channel2_LAN2);
    create_relay_info(&relay_info_LAN2_CHANNEL1, mcast_LAN2_CHANNEL1_mrecv, mcast_LAN1_CHANNEL1_msend, channel1_LAN2, channel1_LAN1);
    create_relay_info(&relay_info_LAN2_CHANNEL2, mcast_LAN2_CHANNEL2_mrecv, mcast_LAN1_CHANNEL2_msend, channel2_LAN2, channel2_LAN1);
    
    // start the relay threads
    pthread_create(LAN1_CHANNEL1_thread, NULL, relay_thread, (void *)relay_info_LAN1_CHANNEL1);
    pthread_create(LAN1_CHANNEL2_thread, NULL, relay_thread, (void *)relay_info_LAN1_CHANNEL2);
    pthread_create(LAN2_CHANNEL1_thread, NULL, relay_thread, (void *)relay_info_LAN2_CHANNEL1);
    pthread_create(LAN2_CHANNEL2_thread, NULL, relay_thread, (void *)relay_info_LAN2_CHANNEL2);
}

/**
 * @brief Shutdown the relay
*/
void shutdown_relay() {
    // stop the relay
    keep_running = 0;

    // wait for the threads to finish
    pthread_join(*LAN1_CHANNEL1_thread, NULL);
    pthread_join(*LAN1_CHANNEL2_thread, NULL);
    pthread_join(*LAN2_CHANNEL1_thread, NULL);
    pthread_join(*LAN2_CHANNEL2_thread, NULL);

    // free the memory allocated for the threads
    free(LAN1_CHANNEL1_thread);
    free(LAN1_CHANNEL2_thread);
    free(LAN2_CHANNEL1_thread);
    free(LAN2_CHANNEL2_thread);

    // free the memory allocated for the relay info structs
    free(relay_info_LAN1_CHANNEL1);
    free(relay_info_LAN1_CHANNEL2);
    free(relay_info_LAN2_CHANNEL1);
    free(relay_info_LAN2_CHANNEL2);

    // destroy the multicast groups
    multicast_destroy(mcast_LAN1_CHANNEL1_mrecv);
    multicast_destroy(mcast_LAN1_CHANNEL1_msend);
    multicast_destroy(mcast_LAN1_CHANNEL2_mrecv);
    multicast_destroy(mcast_LAN1_CHANNEL2_msend);
}

/**
 * @brief Relay the multicast messages between LAN1 and LAN2
 * @todo should this be implemented like a library instead of a main function?
*/
/*int main() {
    // initialize the multicast groups for LAN1 and LAN2
    relay_init("224.1.1.1", "224.1.1.2", 14500, "224.1.1.3", "224.1.1.4", 14500);
    sleep(30); // sleep for 30 seconds
    shutdown_relay(); // shutdown the relay

    return 0;
}*/
