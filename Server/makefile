CC = gcc
CFLAGS = -Wall -Wextra

server: server.o common.o
	$(CC) $(CFLAGS) -o server server.o common.o

server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

clean:
	rm -f *.o server
