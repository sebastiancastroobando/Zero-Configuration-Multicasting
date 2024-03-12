#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_B_CHANNEL1, LAN_B_CHANNEL2, LAN_B_PORT);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "service2_LAN-B"},
        { .attr_name = "location", .value = "kitchen"},
        { .attr_name = "make", .value = "ninja"},
        { .attr_name = "model", .value = "blender-1000"}
    };
    rv = zcs_start("service2_LAN-B", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting service2_LAN-B\n");
    for (int i = 0; i < 5; i++) {
        rv = zcs_post_ad("blend status", "service2_LAN-B posted ad");
        sleep(3);
    }
    printf("Shutting down service2_LAN-B\n");
    rv = zcs_shutdown();
    return rv;
}
