#include <unistd.h>
#include <stdio.h>
#include "../../../../zcs.h"

#define CHANNEL1            "224.1.1.3"
#define CHANNEL2            "224.1.1.4"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, CHANNEL1, CHANNEL2, 14500);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "smart-blender"},
        { .attr_name = "location", .value = "kitchen"},
        { .attr_name = "make", .value = "ninja"},
        { .attr_name = "model", .value = "blender-1000"}
    };
    rv = zcs_start("ninja-blender", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Started the ninja blender\n");
    for (int i = 0; i < 5; i++) {
        rv = zcs_post_ad("blend status", "Ready to make delicious smoothies!");
        sleep(3);
    }
    printf("Shutting down ninja blender\n");
    rv = zcs_shutdown();
    return rv;
}