#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "service1_LAN-A"},
        { .attr_name = "location", .value = "living room"},
        { .attr_name = "make", .value = "google"},
        { .attr_name = "model", .value = "X3K23FADJ23"},
        { .attr_name = "year", .value = "2023"}
    };
    rv = zcs_start("service1_LAN-A", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting service1_LAN-A\n");
    for (int i = 0; i < 10; i++) {
        rv = zcs_post_ad("cast-status", "service1_LAN-A posted ad");
        sleep(1);
    }
    printf("Shutting down service1_LAN-A\n");
    rv = zcs_shutdown();
    return rv;
}
