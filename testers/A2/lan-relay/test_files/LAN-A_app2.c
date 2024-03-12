#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

void ad_callback(char *s, char *r) {
    printf("Received ad in LAN-A! Ad received: %s, with value: %s\n", s, r);
}

int main() {
    int rv;
    printf("Starting app2 in LAN-A\n");
    rv = zcs_init(ZCS_APP_TYPE, LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT);
    char *names[10];
    rv = zcs_query("type", "service2_LAN-B", names, 10);

    if (rv > 0) {
        zcs_attribute_t attrs[5];
        int anum = 5;
        rv = zcs_get_attribs(names[0], attrs, &anum);
        if ((strcmp(attrs[1].attr_name, "location") == 0) && (strcmp(attrs[1].value, "bedroom") == 0)) {
            printf("Listening for ads from [%s] in LAN-A...\n", names[0]);
            rv = zcs_listen_ad(names[0], ad_callback);
        }
        sleep(30);
    }
    printf("Shutting down app2 in LAN-A\n");
    zcs_shutdown();

}
