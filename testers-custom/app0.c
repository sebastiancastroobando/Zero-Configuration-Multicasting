#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../zcs.h"
#include "../multicast.h"

char buffer[100];

void hello(char *s, char *r) {
	printf("Ad received: %s, with value: %s\n", s, r);
	zcs_log();
}

int main(int argc, char *argv[]) {
	int rv;
    rv = zcs_init(ZCS_APP_TYPE);
	/*while(1) {
		sleep(10);
		int num = 5;
		zcs_attribute_t attrs[num];
		zcs_get_attribs("speaker-X", attrs, &num);
		for (int i = 0; i < num; i++)
			printf("value: %s\n", attrs[i].value);
	}*/
	//zcs_listen_ad("speaker-X", hello);
	sleep(20);
	printf("GGGGGGGGGGGGGGGGGGGG\n");
    zcs_shutdown();
}
