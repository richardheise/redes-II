CFLAGS=-Wall

all: client server

client: client.c
	gcc client.c -o client $(CFLAGS)

server: server.c
	gcc server.c -o server $(CFLAGS)

clean:
	rm -f server client
