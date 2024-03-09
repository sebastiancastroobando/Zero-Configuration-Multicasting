#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"

#define CHANNEL1            "224.1.1.3"
#define CHANNEL2            "224.1.1.4"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, CHANNEL1, CHANNEL2, 17500);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "smart-light-bulb"},
        { .attr_name = "location", .value = "basement"},
        { .attr_name = "make", .value = "philips"},
        { .attr_name = "model", .value = "Hue-1000"},
        { .attr_name = "year", .value = "2020"}
    };
    rv = zcs_start("SmartLightBulb", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Started Smart Light Buld\n");
    for (int i = 0; i < 5; i++) {
        rv = zcs_post_ad("light-color", "Light ready to change, in basement with Smart Light Bulb!");
        sleep(5);
    }
    printf("Shutting down Smart Light Buld\n");
    rv = zcs_shutdown();
    return rv;
}