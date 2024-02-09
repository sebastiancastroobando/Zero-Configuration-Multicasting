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
		int num = 5;
		zcs_attribute_t attrs[num];
		zcs_get_attribs("speaker-X", attrs, &num);
		for (int i = 0; i < num; i++)
			printf("value: %s\n", attrs[i].value);
	}
    zcs_shutdown();
}
