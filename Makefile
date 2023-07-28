TARGET = ccc
OBJS = codegen.o error.o main.o parser.o tokenizer.o type.o

CC = gcc
CFLAGS = -Wall -g -std=c17

$(TARGET): $(OBJS)
	$(CC) -static -o $@ $(OBJS) $(LDFLAGS)

.PHONY: test
test: $(TARGET)
	./$(TARGET) test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)

.PHONY: build-gen1
build-gen1: $(TARGET)
	./preprocessor.sh > tmp.c
	./$(TARGET) tmp.c > tmp.s
	$(CC) -static -o ccc-gen1 tmp.s

.PHONY: test-gen1
test-gen1: build-gen1
	./ccc-gen1 test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp

.PHONY: build-gen2
build-gen2: build-gen1
	./preprocessor.sh > tmp.c
	./ccc-gen1 tmp.c > tmp.s
	$(CC) -static -o ccc-gen2 tmp.s

.PHONY: test-gen2
test-gen2: build-gen2
	./ccc-gen2 test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp
