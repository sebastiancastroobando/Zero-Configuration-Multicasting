CC = gcc
CUSTOM_TESTER_DIR = custom-tester/
PROVIDED_TESTER_DIR = provided-testers/

# Files in the parent directory
PARENT_FILES = zzcs.c multicast.c

# Updated file paths with parent directory files added where necessary
TEST_FILES = multicast.c $(PROVIDED_TESTER_DIR)tester.c
SEND_FILES0 = multicast.c zzcs.c $(CUSTOM_TESTER_DIR)service0.c
SEND_FILES1 = multicast.c zzcs.c $(CUSTOM_TESTER_DIR)service1.c
SEND_FILES2 = multicast.c zzcs.c $(CUSTOM_TESTER_DIR)service2.c
RECV_FILES = multicast.c zzcs.c $(PROVIDED_TESTER_DIR)app.c

TEST_TARGET = tester
SEND_TARGET0 = service0
SEND_TARGET1 = service1
SEND_TARGET2 = service2
RECV_TARGET = app

all: test send0 send1 send2 recv

test: $(TEST_FILES)
	$(CC) $(TEST_FILES) -o $(TEST_TARGET)

send0: $(SEND_FILES0)
	$(CC) $(SEND_FILES0) -o $(SEND_TARGET0)

send1: $(SEND_FILES1)
	$(CC) $(SEND_FILES1) -o $(SEND_TARGET1)

send2: $(SEND_FILES2)
	$(CC) $(SEND_FILES2) -o $(SEND_TARGET2)

recv: $(RECV_FILES)
	$(CC) $(RECV_FILES) -o $(RECV_TARGET)

clean:
	rm -f $(TEST_TARGET) $(SEND_TARGET0) $(SEND_TARGET1) $(SEND_TARGET2) $(RECV_TARGET)
