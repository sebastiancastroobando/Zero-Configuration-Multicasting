#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT1, LAN_A_PORT2);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "chromecast"},
        { .attr_name = "location", .value = "living room"},
        { .attr_name = "make", .value = "google"},
        { .attr_name = "model", .value = "X3K23FADJ23"},
        { .attr_name = "year", .value = "2023"}
    };
    rv = zcs_start("chromecast", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting chromecast - service1 in LAN-A\n");
    for (int i = 0; i < 10; i++) {
        rv = zcs_post_ad("cast-status", "Ready to cast on LAN-A with new chromecast");
        sleep(1);
    }
    printf("Shutting down chromecast - service1 in LAN-A\n");
    rv = zcs_shutdown();
    return rv;
}
