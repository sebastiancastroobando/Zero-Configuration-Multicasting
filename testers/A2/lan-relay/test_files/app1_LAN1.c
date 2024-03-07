#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../../zcs.h"

#define CHANNEL1            "224.1.1.1"
#define CHANNEL2            "224.1.1.2"

void ad_callback(char *s, char *r) {
    printf("Received ad in LAN1! Ad received: %s, with value: %s\n", s, r);
}

int main() {
    int rv;
    rv = zcs_init(ZCS_APP_TYPE, CHANNEL1, CHANNEL2, 14500);
    char *names[10];
    rv = zcs_query("type", "chromecast", names, 10);

    if (rv > 0) {
        zcs_attribute_t attrs[5];
        int anum = 5;
        rv = zcs_get_attribs(names[0], attrs, &anum);
        if ((strcmp(attrs[1].attr_name, "location") == 0) && (strcmp(attrs[1].value, "living room") == 0)) {
            printf("Listening for ads in LAN1...\n");
            rv = zcs_listen_ad(names[0], ad_callback);
        }
        sleep(30);
    }
    printf("Shutting down app1\n");
    zcs_shutdown();

}
