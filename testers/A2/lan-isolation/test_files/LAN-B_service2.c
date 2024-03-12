#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_B_CHANNEL1, LAN_B_CHANNEL2, LAN_B_PORT1, LAN_B_PORT2);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "smart-blender"},
        { .attr_name = "location", .value = "kitchen"},
        { .attr_name = "make", .value = "ninja"},
        { .attr_name = "model", .value = "blender-1000"}
    };
    rv = zcs_start("ninja-blender", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting the ninja-blender - service2 in LAN-B\n");
    for (int i = 0; i < 5; i++) {
        rv = zcs_post_ad("blend status", "Ready to make delicious smoothies! - LAN-B");
        sleep(3);
    }
    printf("Shutting down ninja-blender - service2 in LAN-B\n");
    rv = zcs_shutdown();
    return rv;
}
