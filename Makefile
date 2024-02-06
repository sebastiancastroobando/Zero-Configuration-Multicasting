CC = gcc
TEST_FILES = multicast.c tester.c
SEND_FILES = multicast.c zcs.c seb_service.c
RECV_FILES = multicast.c zcs.c seb_app.c
TEST_TARGET = tester
SEND_TARGET0 = seb_service0
SEND_TARGET1 = seb_service1
SEND_TARGET2 = seb_service2
RECV_TARGET = seb_app

all: test send recv

test: $(TEST_FILES)
	$(CC) $(TEST_FILES) -o $(TEST_TARGET)
send: $(SEND_FILES)
	$(CC) $(SEND_FILES) -o $(SEND_TARGET0)
	$(CC) $(SEND_FILES) -o $(SEND_TARGET1)
	$(CC) $(SEND_FILES) -o $(SEND_TARGET2)
recv: $(RECV_FILES)
	$(CC) $(RECV_FILES) -o $(RECV_TARGET)
clean:
	rm $(TEST_TARGET) $(SEND_TARGET0) $(SEND_TARGET1) $(SEND_TARGET2) $(RECV_TARGET)
