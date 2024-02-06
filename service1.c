#include <unistd.h>
#include "zcs.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE);
    zcs_attribute_t attribs[] = {
	    { .attr_name = "type", .value = "washer-dryer"},
        { .attr_name = "location", .value = "kitchen"},
        { .attr_name = "make", .value = "whirlpool"},
        { .attr_name = "model", .value = "WFW75HEFW"} };
    rv = zcs_start("Whirlpool WD", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));

    
    zcs_shutdown();
    return 0;
}