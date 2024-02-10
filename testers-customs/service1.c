#include <unistd.h>
#include "../zcs.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE);
    zcs_attribute_t attribs[] = {
	    { .attr_name = "type", .value = "router"},
	    { .attr_name = "location", .value = "living room"},
	    { .attr_name = "make", .value = "linksys"} };
    rv = zcs_start("my-router", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    for (int i = 0; i < 1000; i++) {
        rv = zcs_post_ad("mute", "on");
        sleep(10);
        rv = zcs_post_ad("mute", "off");
        sleep(10);
    }
    rv = zcs_shutdown();
}