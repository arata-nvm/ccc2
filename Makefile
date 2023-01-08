TARGET = ccc
OBJS = codegen.o error.o main.o parser.o tokenizer.o type.o

CC = gcc
CFLAGS = -Wall -g -std=c17

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: test
test: $(TARGET)
	./$(TARGET) test.c > tmp.s
	$(CC) tmp.s -o tmp
	./tmp

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
