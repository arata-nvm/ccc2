TARGET = ccc
OBJS = codegen.o error.o main.o parser.o tokenizer.o type.o

CC = gcc
CFLAGS = -Wall -g -std=c17

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: test
test: $(TARGET)
	./test.sh

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
