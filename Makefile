TARGET = ccc
OBJS = main.o

CC = gcc
CFLAGS = -Wall -g -std=c17

$(TARGET): $(OBJS)

.PHONY: clean
clean:
	rm -rf *.o
