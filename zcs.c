#include <stdlib.h>
#include "zcs.h"
#include "multicast.h"

// DEFAULT PORT NUMBERS : 
#define SRC_PORT : 16000
#define DEST_PORT : 16001

int zcs_init() {
    // TODO : What ports to use?
    mcast_t *m multicast_init("224.1.1.1", 10000, 10001);
    return 0;
}

int zcs_start(char *name, zcs_attribute_t attr[], int num) {
	// called by server nodes only?
	// 	if so, use this for server specific attributes
	return 0;
}

int zcs_post_ad(char *ad_name, char *ad_value) {
	return 0;
}

int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen) {
	// namelen for attr_name?
	// called by application nodes only?
	// 	if so, then use this for app node specific attributes
	return 0;
}

int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num) {
	return 0;
}

int zcs_listen_ad(char *name, zcs_cb_f cback) {
	return 0;
}

int zcs_shutdown() {
	// Pretty sure this is the shutdown for the whole library
	// and the bit about a node not starting after shutdown is
	// likely about needing to call init before creating nodes
	// not sure why they put that in the handout
	return 0;
}

void zcs_log() {
}
