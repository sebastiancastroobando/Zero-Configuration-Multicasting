#include "../multicast.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../zcs.h"

char buffer[100];

int main(int argc, char *argv[]) {
	int rv;
    rv = zcs_init(ZCS_APP_TYPE);
    
    // rv = zcs_query("type", "speaker", names, 10);
    //if (rv > 0) {
    //    zcs_attribute_t attrs[5];
	//    int anum = 5;
    //    rv = zcs_get_attribs(names[0], attrs, &anum);
    //    if ((strcmp(attrs[0].attr_name, "location") == 0) && (strcmp(attrs[0].value, "kitchen") == 0)) {
    //            rv = zcs_listen_ad(names[0], hello);
    //    }
    //}

    
    zcs_shutdown();
}
