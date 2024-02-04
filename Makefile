CC = gcc
TEST_FILES = multicast.c tester.c
SEND_FILES = multicast.c zcs.c Seb-testers/seb_service.c
RECV_FILES = multicast.c zcs.c Seb-testers/seb_app.c
TEST_TARGET = tester
SEND_TARGET = seb_service
RECV_TARGET = seb_app

all: test send recv

test: $(TEST_FILES)
	$(CC) $(TEST_FILES) -o $(TEST_TARGET)
send: $(SEND_FILES)
	$(CC) $(SEND_FILES) -o $(SEND_TARGET)
recv: $(RECV_FILES)
	$(CC) $(RECV_FILES) -o $(RECV_TARGET)
clean:
	rm $(TEST_TARGET) $(SEND_TARGET) $(RECV_TARGET)
