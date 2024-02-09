#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "zcs.h"
#include "multicast.h"

char buffer[100];

int main(int argc, char *argv[]) {
	int rv;
    rv = zcs_init(ZCS_APP_TYPE);
	while(1) {
		sleep(10);
		zcs_log();
	}
    zcs_shutdown();
}
