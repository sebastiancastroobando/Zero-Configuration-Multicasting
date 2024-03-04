#include <unistd.h>
#include <stdio.h>
#include "../zcs.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE);
    zcs_attribute_t attribs[] = {
	    { .attr_name = "type", .value = "router"},
	    { .attr_name = "location", .value = "living room"},
	    { .attr_name = "make", .value = "linksys"} };
    rv = zcs_start("my-router", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    for (int i = 0; i < 1; i++) {
        printf("muting ad\n");
        rv = zcs_post_ad("mute", "on");
        sleep(5);
        printf("unmuting ad\n");
        rv = zcs_post_ad("mute", "off");
        sleep(5);
    }
    rv = zcs_shutdown();
}
