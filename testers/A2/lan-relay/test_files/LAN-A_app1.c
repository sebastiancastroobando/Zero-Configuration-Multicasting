#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../../../../zcs.h"
#include "../../../../relay.h"

void ad_callback(char *s, char *r) {
    printf("Received ad in LAN-A! Ad received: %s, with value: %s\n", s, r);
}

void ad_callback1(char *s, char *r) {
    printf("Received ad in LAN-A! %s, with value: %s\n", s, r);
}

int main() {
    int rv;
    printf("Starting app1 in LAN-A\n");
    rv = zcs_init(ZCS_APP_TYPE, LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT);
    relay_init(LAN_A_CHANNEL1, LAN_A_CHANNEL2, LAN_A_PORT, LAN_B_CHANNEL1, LAN_B_CHANNEL2, LAN_B_PORT);
    char *names[10];
    char *names1[10];
    rv = zcs_query("type", "chromecast", names, 10);
    rv = zcs_query("type", "smart-light-bulb", names1, 10);
    printf("rv after second query is: %d\n", rv);
    if (rv > 0) {
        zcs_attribute_t attrs[5];
        int anum = 5;
        rv = zcs_get_attribs(names[0], attrs, &anum);
        zcs_attribute_t attrs1[5];
        int anum1 = 5;
        rv = zcs_get_attribs(names1[0], attrs1, &anum1);
        if ((strcmp(attrs[1].attr_name, "location") == 0) && (strcmp(attrs[1].value, "living room") == 0)) {
            printf("Listening for ads from [%s] in LAN-A...\n", names[0]);
            rv = zcs_listen_ad(names[0], ad_callback);
        }
        if ((strcmp(attrs1[1].attr_name, "location") == 0) && (strcmp(attrs1[1].value, "basement") == 0)) {
            printf("Listening for ads from [%s] in LAN-A...\n", names1[0]);
            rv = zcs_listen_ad(names1[0], ad_callback1);
        }
        sleep(30);
    }
    printf("Shutting down app1 in LAN-A\n");
    shutdown_relay();
    zcs_shutdown();

}
