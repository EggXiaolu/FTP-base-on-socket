CC = gcc
CFLAGS = -Wall -Wextra

client: client.o common.o
	$(CC) $(CFLAGS) -o client client.o common.o

client.o: client.c common.h
	$(CC) $(CFLAGS) -c client.c

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c

clean:
	rm -f *.o client
