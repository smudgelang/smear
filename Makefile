CC=gcc
CFLAGS=-g -std=c89 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -pthread

default: lights

.PHONY: clean

all: libsmear.a

lib%.a: queue.o smear.o
	ar -cvq $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.a
