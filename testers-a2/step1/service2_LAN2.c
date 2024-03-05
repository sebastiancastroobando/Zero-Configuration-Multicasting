#include <unistd.h>
#include <stdio.h>
#include "../../zcs.h"

#define CHANNEL1            "224.1.1.3"
#define CHANNEL2            "224.1.1.4"

int main() {
    int rv;
    rv = zcs_init(ZCS_SERVICE_TYPE, CHANNEL1, CHANNEL2, 14500);
    zcs_attribute_t attribs[] = 
    {
        { .attr_name = "type", .value = "chromecast"},
        { .attr_name = "location", .value = "living room"},
        { .attr_name = "make", .value = "google"},
        { .attr_name = "model", .value = "U4A23TEDJ15"},
        { .attr_name = "year", .value = "2019"}
    };
    rv = zcs_start("old_chromecast", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    printf("Started chromecast\n");
    for (int i = 0; i < 5; i++) {
        rv = zcs_post_ad("cast-status", "Ready to cast on LAN2 with old chromecast");
        sleep(5);
    }
}