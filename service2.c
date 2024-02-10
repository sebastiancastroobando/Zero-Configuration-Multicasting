#include <unistd.h>
#include <stdio.h>
#include "zcs.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE);
    zcs_attribute_t attribs[] = {
        { .attr_name = "type", .value = "Smart Christmas Tree"},
        { .attr_name = "location", .value = "Living Room"},
        { .attr_name = "make", .value = "Ikea"},
        { .attr_name = "model", .value = "FJALLBO"},
        { .attr_name = "led-color", .value = "Green"} };
    rv = zcs_start("my-tree", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    for (int i = 0; i < 1000; i++) {
        printf("sending ad...\n");
        rv = zcs_post_ad("mute", "on");
        sleep(10);
        rv = zcs_post_ad("mute", "off");
        sleep(10);
    }
    rv = zcs_shutdown();
}
