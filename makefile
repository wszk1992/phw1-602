CC = gcc
DEBUG = -g
CFLAGS = -Wall -lpthread -c $(DEBUG)
LFLAGS = -Wall -lpthread $(DEBUG)

all: team8_client team8_server

team8_client: team8_client.o chatroom_utils.o
	$(CC) $(LFLAGS) team8_client.o chatroom_utils.o -o team8_client

team8_server: team8_server.o chatroom_utils.o
	$(CC) $(LFLAGS) team8_server.o chatroom_utils.o -o team8_server


team8_client.o: team8_client.c chatroom_utils.h
	$(CC) $(CFLAGS) team8_client.c

team8_server.o: team8_server.c chatroom_utils.h
	$(CC) $(CFLAGS) team8_server.c

chatroom_utils.o: chatroom_utils.h chatroom_utils.c
	$(CC) $(CFLAGS) chatroom_utils.c

clean:
	rm -rf *.o *~ team8_client team8_server
