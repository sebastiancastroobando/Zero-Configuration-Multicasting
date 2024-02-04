#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "../zcs.h"

int ZCS_APP_TYPE = 1; // TODO: remove this, just for testing

void hello(char *s, char *r) {
    printf("Ad received: %s, with value: %s\n", s, r);
    zcs_log();
}

int main() {
    int rv;
    rv = zcs_init(ZCS_APP_TYPE);
}


