CC = gcc
CFLAGS = -Wall -Wextra -O3 -fPIC -pthread -Iinclude
LDFLAGS = -shared -pthread
TARGET = libhmalloc.so

SRCS = src/hmalloc.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f tests/test_basic tests/test_threads

test: all
	$(CC) $(CFLAGS) -o tests/test_basic tests/test_basic.c -L. -lhmalloc -Wl,-rpath,.
	$(CC) $(CFLAGS) -o tests/test_threads tests/test_threads.c -L. -lhmalloc -Wl,-rpath,.
	./tests/test_basic
	./tests/test_threads

.PHONY: all clean test
