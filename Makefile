CC = gcc
CUSTOM_TESTER_DIR = testers-custom/
PROVIDED_TESTER_DIR = testers-provided/

# Files in the parent directory
PARENT_FILES = zcs.c multicast.c

# Updated file paths with parent directory files added where necessary
TEST_FILES = multicast.c $(PROVIDED_TESTER_DIR)tester.c
RECV_FILES0 = multicast.c zcs.c $(CUSTOM_TESTER_DIR)app0.c
SEND_FILES0 = multicast.c zcs.c $(CUSTOM_TESTER_DIR)service0.c
SEND_FILES1 = multicast.c zcs.c $(CUSTOM_TESTER_DIR)service1.c
SEND_FILES2 = multicast.c zcs.c $(CUSTOM_TESTER_DIR)service2.c
SEND_FILES = multicast.c zcs.c $(PROVIDED_TESTER_DIR)service.c
RECV_FILES = multicast.c zcs.c $(PROVIDED_TESTER_DIR)app.c

TEST_TARGET = tester
RECV_TARGET0 = app0
SEND_TARGET0 = service0
SEND_TARGET1 = service1
SEND_TARGET2 = service2
SEND_TARGET = service
RECV_TARGET = app

all: test recv0 send0 send1 send2 send recv

test: $(TEST_FILES)
	$(CC) $(TEST_FILES) -o $(TEST_TARGET)

recv0: $(RECV_FILES0)
	$(CC) $(RECV_FILES0) -o $(RECV_TARGET0)

send0: $(SEND_FILES0)
	$(CC) $(SEND_FILES0) -o $(SEND_TARGET0)

send1: $(SEND_FILES1)
	$(CC) $(SEND_FILES1) -o $(SEND_TARGET1)

send2: $(SEND_FILES2)
	$(CC) $(SEND_FILES2) -o $(SEND_TARGET2)

send: $(SEND_FILES)
	$(CC) $(SEND_FILES) -o $(SEND_TARGET)

recv: $(RECV_FILES)
	$(CC) $(RECV_FILES) -o $(RECV_TARGET)

clean:
	rm -f $(TEST_TARGET) $(RECV_TARGET0) $(SEND_TARGET0) $(SEND_TARGET1) $(SEND_TARGET2) $(SEND_TARGET) $(RECV_TARGET)
