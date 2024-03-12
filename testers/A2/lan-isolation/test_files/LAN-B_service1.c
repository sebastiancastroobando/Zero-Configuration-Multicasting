#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, LAN_B_CHANNEL1, LAN_B_CHANNEL2, LAN_B_PORT1, LAN_B_PORT2);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "smart-light-bulb"},
        { .attr_name = "location", .value = "basement"},
        { .attr_name = "make", .value = "philips"},
        { .attr_name = "model", .value = "Hue-1000"},
        { .attr_name = "year", .value = "2020"}
    };
    rv = zcs_start("SmartLightBulb", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Starting SmartLightBulb - service1 in LAN-B\n");
    for (int i = 0; i < 5; i++) {
        rv = zcs_post_ad("light-color", "Light ready to change, in basement with Smart Light Bulb! - in LAN-B");
        sleep(5);
    }
    printf("Shutting down SmartLightBulb - service1 in LAN-B\n");
    rv = zcs_shutdown();
    return rv;
}
