#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "zcs.h"

int ZCS_APP_TYPE = 1; // TODO: remove this, just for testing

void hello(char *s, char *r) {
    printf("Ad received: %s, with value: %s\n", s, r);
    zcs_log();
}

int main() {
    int rv;
    rv = zcs_init(ZCS_APP_TYPE);
    char *names[10];
    rv = zcs_query("type", "speaker", names, 10);
    if (rv > 0) {
        zcs_attribute_t attrs[5];
	    int anum = 5;
        rv = zcs_get_attribs(names[0], attrs, &anum);
        if ((strcmp(attrs[0].attr_name, "location") == 0) && (strcmp(attrs[0].value, "kitchen") == 0)) {
                rv = zcs_listen_ad(names[0], hello);
        }
    }
}

/*
char *key, *value;
				split_key_value(msg, &key, &value);
				if (strcmp(key, "nodeName") == 0 && strcmp(value, name) == 0) {
					// call the callback function
					split_key_value(msg, &key, &value);
					if (strcmp(key, "adName") == 0) {
						strcpy(ad_name, value);
					}
					split_key_value(msg, &key, &value);
					if (strcmp(key, "adValue") == 0) {
						strcpy(ad_value, value);
					}
					cback(ad_name, ad_value);
				}

*/