#ifndef __ZCS_H__
#define __ZCS_H__

typedef struct {
    char *attr_name;
    char *value;
} zcs_attribute_t;

typedef void (*zcs_cb_f)(char *, char *);


#define ZCS_APP_TYPE		1
#define ZCS_SERVICE_TYPE	2

// We need to know the port number of the ZCS multicast group
#define ZCS_PORT			14500
#define ZCS_PORT1			17500
// #define ZCS_CHANNEL1		"224.1.1.1"
// #define ZCS_CHANNEL2		"224.1.1.2"

#define MAX_NAME_LEN		64      // max length of a node name
#define MAX_AD_DURATION		2      // in seconds
#define MAX_AD_ATTEMPTS		1       // number of attempts to send an ad
#define HEARTBEAT_INTERVAL	5       // in seconds
#define BUF_SIZE			1000    // TODO : 100 seems like too little, let's talk about this.
#define MAX_SIZE			10      // max number of nodes
#define MAX_MSG_SIZE		20      // max number of {attr_name, value} pairs
#define LOG_SIZE			6		// max number of log entries per node (UP/DOWN)

// Verbose flag
#define VERBOSE             1      // 1 for verbose, 0 for non-verbose

int zcs_init(int type, char *channel1, char *channel2, int port);
int zcs_start(char *name, zcs_attribute_t attr[], int num);
int zcs_post_ad(char *ad_name, char *ad_value);
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen);
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num);
int zcs_listen_ad(char *name, zcs_cb_f cback);
int zcs_shutdown();
void zcs_log();

#endif

