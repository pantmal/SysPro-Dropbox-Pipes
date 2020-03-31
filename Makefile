CC=gcc

CFLAGS=-c
all: mirror_client

mirror_client: List.o mirror_client.o
	$(CC) -o mirror_client List.o mirror_client.o

List.o: List.c 
	$(CC) $(CFLAGS) List.c

mirror_client.o: mirror_client.c
	$(CC) $(CFLAGS) mirror_client.c
clean:
	rm -rf *o mirror_client
