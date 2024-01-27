CC = gcc
TEST_FILES = multicast.c tester.c
TEST_TARGET = tester

test: $(TEST_FILES)
	$(CC) $(TEST_FILES) -o $(TEST_TARGET)
clean: $(TEST_TARGET)
	rm $(TEST_TARGET)
