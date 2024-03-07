#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "multicast.h"

#define MAX_MSG_SIZE 1000 // maximum size of the multicast message to be relayed

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
 * @todo should we add some formatting? Like : {sourceLAN:LAN1; sourceChannel:channel1; destinationLAN:LAN2; destinationChannel:channel1}
*/
void *relay_thread(void *arg) {
    // The relay thread will take one receiver from a channel and send it to the other channel with a sender.
    
    // get the multicast receiver and sender objects
    mcast_t *mrecv = (mcast_t *)arg;
    mcast_t *msend = (mcast_t *)arg;
    // also name of 

    char received_data[MAX_MSG_SIZE];

    // receive the multicast message
    while(keep_running) {
        if (multicast_check_receive(mrecv) > 0) {
            multicast_receive(mrecv, received_data, MAX_MSG_SIZE); // receive the multicast message

            // check if the received message is already relayed
            if (strstr(received_data, "relayed:true") != NULL) {
                continue; // if the message is already relayed, then skip it
            }

            // Concatenate "relayed:true" to the received message
            strcat(received_data, "relayed:true");

            printf("Relaying message: %s\n", received_data);
        }
    }
}

/**
 * @brief Create struct to hold the multicast receiver and sender objects
*/
void create_relay_info(mcast_t *mrecv, mcast_t *msend, char *channel_source, char *channel_destination) {
    relay_info_t *relay_info = (relay_info_t *)malloc(sizeof(relay_info_t));
    relay_info->mrecv = mrecv;
    relay_info->msend = msend;
    relay_info->channel_source = channel_source;
    relay_info->channel_destination = channel_destination;
}

/**
 * @brief Initialize the multicast groups for LAN1 and LAN2
*/
void reley_init(char *channel1_LAN1, char *channel2_LAN1, int port_LAN1, char *channel1_LAN2, char *channel2_LAN2, int port2_LAN2) {
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

    // create the relay info structs
    create_relay_info(mcast_LAN1_CHANNEL1_mrecv, mcast_LAN2_CHANNEL1_msend, "LAN1_CHANNEL1", "LAN2_CHANNEL1");
    create_relay_info(mcast_LAN1_CHANNEL2_mrecv, mcast_LAN2_CHANNEL2_msend, "LAN1_CHANNEL2", "LAN2_CHANNEL2");
    create_relay_info(mcast_LAN2_CHANNEL1_mrecv, mcast_LAN1_CHANNEL1_msend, "LAN2_CHANNEL1", "LAN1_CHANNEL1");
    create_relay_info(mcast_LAN2_CHANNEL2_mrecv, mcast_LAN1_CHANNEL2_msend, "LAN2_CHANNEL2", "LAN1_CHANNEL2");

    // start the relay threads
    pthread_create(LAN1_CHANNEL1_thread, NULL, relay_thread, mcast_LAN1_CHANNEL1_mrecv);
    pthread_create(LAN1_CHANNEL2_thread, NULL, relay_thread, mcast_LAN1_CHANNEL2_mrecv);
    pthread_create(LAN2_CHANNEL1_thread, NULL, relay_thread, mcast_LAN2_CHANNEL1_mrecv);
    pthread_create(LAN2_CHANNEL2_thread, NULL, relay_thread, mcast_LAN2_CHANNEL2_mrecv);
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
int main() {
    // initialize the multicast groups for LAN1 and LAN2
    reley_init("224.1.1.1", "224.1.1.2", 14500, "224.1.1.3", "224.1.1.4", 14500);
    sleep(30); // sleep for 30 seconds
    shutdown_relay(); // shutdown the relay

    return 0;
}