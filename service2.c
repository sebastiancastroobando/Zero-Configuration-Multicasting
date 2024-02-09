#include <unistd.h>
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
    rv = zcs_start("Big Ol' Decorations LTD", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));

    
    zcs_shutdown();
    return 0;
}
