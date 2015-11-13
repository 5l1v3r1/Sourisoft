CC=gcc
CFLAGS=-Wall -lssh

client: client.o ConnectAuth.o
	$(CC) $(CFLAGS)  client.o ConnectAuth.o -o client

client.o:client.c
	$(CC) $(CFLAGS) -c  client.c

ConnectAuth.o: ConnectAuth.c ConnectAuth.h
	$(CC) -c ConnectAuth.c $(CFLAGS)

clean: 
	rm *o client *~
