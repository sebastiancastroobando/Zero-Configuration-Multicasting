#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "service2_LAN-B"},
        { .attr_name = "location", .value = "bedroom"},
        { .attr_name = "xboxOS-version", .value = "2023.2.4"}
    };
    rv = zcs_start("service2_LAN-B", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting service2_LAN-B\n");
    for (int i=0; i < 10; i++) {
        rv = zcs_post_ad("cast-status", "service2_LAN-B posted ad");
        sleep(5);
    }
    printf("Shutting down service2_LAN-B\n");
    rv = zcs_shutdown();
    return rv;
}
