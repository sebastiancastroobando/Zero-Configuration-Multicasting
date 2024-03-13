#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT1, LAN_A_PORT2);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "xbox-cloud-play"},
        { .attr_name = "location", .value = "bedroom"},
        { .attr_name = "xboxOS-version", .value = "2023.2.4"}
    };
    rv = zcs_start("xbox", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting xbox - service2 in LAN-A\n");
    for (int i=0; i < 10; i++) {
        rv = zcs_post_ad("cast-status", "Xbox cloud gaming ready on LAN-A with xbox");
        sleep(5);
    }
    printf("Shutting down xbox - service2 in LAN-A\n");
    rv = zcs_shutdown();
    return rv;
}
