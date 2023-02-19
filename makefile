CFLAGS= -Wall
CSV_FLAG= -DCSV_FORMAT

all: client server server_csv

client: client.c
	gcc client.c -o client $(CFLAGS)

server: server.c
	gcc server.c -o server $(CFLAGS)

server_csv: server.c
	gcc server.c -o server_csv $(CSV_FLAG)

clean:
	rm -f server client server_csv
