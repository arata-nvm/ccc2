TARGET = ccc
OBJS = main.o

CC = gcc
CFLAGS = -Wall -g -std=c17

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
