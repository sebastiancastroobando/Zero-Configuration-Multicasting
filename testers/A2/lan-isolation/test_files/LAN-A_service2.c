#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"

#define CHANNEL1            "224.1.1.1"
#define CHANNEL2            "224.1.1.2"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, CHANNEL1, CHANNEL2, 14500);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "xbox-cloud-play"},
        { .attr_name = "location", .value = "bedroom"},
        { .attr_name = "xboxOS-version", .value = "2023.2.4"}
    };
    rv = zcs_start("xbox", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Xbox Cloud Gaming started \n");
    for (int i=0; i < 10; i++) {
        rv = zcs_post_ad("cast-status", "Xbox cloud gaming ready on LAN1 with xbox");
        sleep(5);
    }
    printf("Shutting down NEW chromecast\n");
    rv = zcs_shutdown();
    return rv;
}