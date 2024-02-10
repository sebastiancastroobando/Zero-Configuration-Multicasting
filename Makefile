CC = gcc
TEST_FILES = multicast.c tester.c
SEND_FILES0 = multicast.c zcs.c service0.c 
SEND_FILES1 = multicast.c zcs.c service1.c
SEND_FILES2 = multicast.c zcs.c service2.c 
RECV_FILES = multicast.c zcs.c app0.c
TEST_TARGET = tester
SEND_TARGET0 = service
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
