#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../../../../zcs.h"

void ad_callback(char *s, char *r) {
    printf("Received ad in LAN-B! Ad received: %s, with value: %s\n", s, r);
}

int main() {
    int rv;
    printf("Starting app1 in LAN-B\n");
    rv = zcs_init(ZCS_APP_TYPE, LAN_B_CHANNEL1, LAN_B_CHANNEL2, LAN_B_PORT);
    char *names[10];
    rv = zcs_query("type", "smart-light-bulb", names, 10);

    if (rv > 0) {
        zcs_attribute_t attrs[5];
        int anum = 5;
        rv = zcs_get_attribs(names[0], attrs, &anum);
        if ((strcmp(attrs[1].attr_name, "location") == 0) && (strcmp(attrs[1].value, "basement") == 0)) {
            printf("Listening for ads from [%s] in LAN-B...\n", names[0]);
            rv = zcs_listen_ad(names[0], ad_callback);
        }
        sleep(30);
    }
    printf("Shutting down app1 in LAN-B\n");
    zcs_shutdown();
}
