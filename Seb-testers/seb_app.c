#include "../multicast.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char buffer[100];

int main(int argc, char *argv[]) {
	int src = 17500;
	int dest = 14500;
    if (argc <= 1) {
		printf("Usage: tester src-port dest-port message \n");
		exit(0);
    }
    
    //printf("Source port: %d\n", atoi(argv[1]));
    //printf("Destination port: %d\n", atoi(argv[2]));

    mcast_t *m = multicast_init("224.1.1.2", src, dest);

    char *msg = argv[1];
    multicast_setup_recv(m);
    printf("==============\n");
    multicast_send(m, msg, strlen(msg));
    printf("==============\n");
    while (1) {
	while (multicast_check_receive(m) == 0) {
	    multicast_send(m, msg, strlen(msg));
	    printf("repeat.. \n");
	}
	multicast_receive(m, buffer, 100);
	printf("Received:  %s\r\n", buffer);
	fflush(stdout);
    }
}
