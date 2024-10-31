CC=gcc
CFLAGS=-Wall -g

all: parallel_sum

parallel_sum: parallel_sum.c sum_lib.o
	$(CC) $(CFLAGS) -o parallel_sum parallel_sum.c sum_lib.o -lpthread

sum_lib.o: sum_lib.c sum_lib.h
	$(CC) $(CFLAGS) -c sum_lib.c

clean:
	rm -f parallel_sum sum_lib.o