CC = g++
LD = ld
CFLAGS = -g -Wall -std=c++11
LDFLAGS = -lpthread

# testing target
TEST_TAR = ../subreactor

# test dependents module
DEV = ../utils.o ../epoll.o

# test code
TEST_SRC = test_task

# unit test
$(TEST_SRC): $(TEST_SRC).o $(TEST_TAR).o
	$(CC) $(CFLAGS) -o $(TEST_SRC) subreactor.o $(TEST_SRC).o $(DEV)

$(TEST_SRC).o: $(TEST_TAR).o $(TEST_TAR).h $(TEST_TAR).cpp $(TEST_SRC).cpp
	$(CC) $(CFLAGS) -c $(TEST_SRC).cpp

$(TEST_TAR).o: $(TEST_TAR).h $(TEST_TAR).cpp
	$(CC) $(CFLAGS) -c $(TEST_TAR).cpp

# client test
# $(TEST_SRC): $(TEST_SRC).o
# 	$(CC) $(CFLAGS) $(TEST_SRC).o $(LDFLAGS) -o $(TEST_SRC) $(DEV)

# $(TEST_SRC).o: $(TEST_SRC).cpp $(DEV)
# 	$(CC) $(CFLAGS) -c $(TEST_SRC).cpp

rm:
	rm test_task *.o
