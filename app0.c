#include "multicast.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zcs.h"

char buffer[100];

int main(int argc, char *argv[]) {
	int rv;
    rv = zcs_init(ZCS_APP_TYPE);
    
    zcs_shutdown();
}
