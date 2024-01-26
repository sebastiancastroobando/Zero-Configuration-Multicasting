#include <unistd.h>
#include "zcs.h"
#include <stdio.h>
#include <string.h>

void hello(char *s, char *r) {
    printf("Ad received: %s, with value: %s\n", s, r);
}

int main() {
    int rv;
    rv = zcs_init();
    char *names[10];
    rv = zcs_query("type", "speaker", names);
    if (rv > 0) {
        zcs_attribute_t attrs[5];
        rv = zcs_get_attribs(names[0], attrs, 5);
        if ((strcmp(attrs[0].attr_name, "location") == 0) &&
            (strcmp(attrs[0].value, "kitchen") == 0)) {
                rv = zcs_listen_ad(names[0], hello);
        }
    }
}


