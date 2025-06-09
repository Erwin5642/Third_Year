CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic

TEST_IP = 127.0.0.1
TEST_PORT = 8888

CLIENT_SOURCES = cliente.c gasStationProtocol.c
SERVER_SOURCES = servidor.c KdTree.c list.c utils.c gasStationProtocol.c

CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)

all: cliente servidor

cliente: $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

servidor: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o cliente servidor

test: test.o gasStationProtocol.o
	$(CC) $(CFLAGS) test.o gasStationProtocol.o -o test
	
test.o: test.c gasStationProtocol.h
	$(CC) $(CFLAGS) -c test.c

run:
	./test $(TEST_IP) $(TEST_PORT)

debug: servidor
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./servidor