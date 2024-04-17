CC=gcc
CFLAGS=-I./src/server/include -I./src/client/include

all: server client

server: src/server/src/server.c
	$(CC) src/server/src/server.c -o server $(CFLAGS)

client: src/client/src/client.c
	$(CC) src/client/src/client.c -o client $(CFLAGS)

.PHONY: clean

clean:
	rm -f server client
