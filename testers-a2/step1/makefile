# Makefile for step1 of testers-a2
CC = gcc

LIBRARY_PATH = ../../

APP1_LAN1 = $(LIBRARY_PATH)zcs.c $(LIBRARY_PATH)multicast.c app1_LAN1.c
APP2_LAN2 = $(LIBRARY_PATH)zcs.c $(LIBRARY_PATH)multicast.c app2_LAN2.c
SERVICE1_LAN1 = $(LIBRARY_PATH)zcs.c $(LIBRARY_PATH)multicast.c service1_LAN1.c
SERVICE2_LAN2 = $(LIBRARY_PATH)zcs.c $(LIBRARY_PATH)multicast.c service2_LAN2.c

APP1_LAN1_TARGET = app1_LAN1
APP2_LAN2_TARGET = app2_LAN2
SERVICE1_LAN1_TARGET = service1_LAN1
SERVICE2_LAN2_TARGET = service2_LAN2

all: app1_LAN1 app2_LAN2 service1_LAN1 service2_LAN2

app1_LAN1: $(APP1_LAN1)
	$(CC) $(APP1_LAN1) -o $(APP1_LAN1_TARGET)

app2_LAN2: $(APP2_LAN2)
	$(CC) $(APP2_LAN2) -o $(APP2_LAN2_TARGET)

service1_LAN1: $(SERVICE1_LAN1)
	$(CC) $(SERVICE1_LAN1) -o $(SERVICE1_LAN1_TARGET)

service2_LAN2: $(SERVICE2_LAN2)
	$(CC) $(SERVICE2_LAN2) -o $(SERVICE2_LAN2_TARGET)

clean:
	rm -f $(APP1_LAN1_TARGET) $(APP2_LAN2_TARGET) $(SERVICE1_LAN1_TARGET) $(SERVICE2_LAN2_TARGET) app1_log.txt app2_log.txt service1_log.txt service2_log.txt