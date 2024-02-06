#include <unistd.h>
#include "zcs.h"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE);
    zcs_attribute_t attribs[] = {
	    { .attr_name = "type", .value = "speaker"},
	    { .attr_name = "location", .value = "kitchen"},
	    { .attr_name = "make", .value = "yamaha"} };
    rv = zcs_start("speaker-X", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    
    
    zcs_shutdown();
    return 0;
}

