#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../zcs.h"

void hello(char *s, char *r) {
	printf("HELLO FROM APP ONE\n");
    printf("Ad received: %s, with value: %s\n", s, r);
}

int main() {
    int rv;
    rv = zcs_init(ZCS_APP_TYPE);
    char *names[10];
    rv = zcs_query("type", "router", names, 10);
	if (rv > 0) {
        zcs_attribute_t attrs[5];

	    int anum = 5;
        rv = zcs_get_attribs(names[0], attrs, &anum);
		printf("node_name: %s\n", names[0]);
        if ((strcmp(attrs[1].attr_name, "location") == 0) && (strcmp(attrs[1].value, "living room") == 0)) {
                rv = zcs_listen_ad(names[0], hello);
        }
		// makes process sleep so that shutdown works properly
		sleep(60);
    }
	zcs_shutdown();
}
