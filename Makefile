CC = gcc
CFLAGS = -Wall -Wextra -O3 -fPIC -pthread -Iinclude
LDFLAGS = -shared -pthread
SHARED_TARGET = libhmalloc.so
STATIC_TARGET = libhmalloc.a

SRCS = src/hmalloc.c
OBJS = $(SRCS:.c=.o)

all: $(SHARED_TARGET) $(STATIC_TARGET)

$(SHARED_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(STATIC_TARGET): $(OBJS)
	ar rcs $@ $^

examples: $(SHARED_TARGET) $(STATIC_TARGET)
	$(CC) $(CFLAGS) -o examples/shared_01_basic examples/shared_01_basic.c
	$(CC) $(CFLAGS) -o examples/shared_02_array examples/shared_02_array.c
	$(CC) $(CFLAGS) -o examples/shared_03_struct examples/shared_03_struct.c
	$(CC) $(CFLAGS) -o examples/shared_04_calloc examples/shared_04_calloc.c
	$(CC) $(CFLAGS) -o examples/shared_05_threads examples/shared_05_threads.c
	$(CC) $(CFLAGS) -o examples/static_01_basic examples/static_01_basic.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_02_array examples/static_02_array.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_03_struct examples/static_03_struct.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_04_large examples/static_04_large.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_05_threads examples/static_05_threads.c libhmalloc.a

clean:
	rm -f $(OBJS) $(SHARED_TARGET) $(STATIC_TARGET)
	rm -f tests/test_basic tests/test_threads
	rm -f examples/shared_01_basic examples/shared_02_array examples/shared_03_struct examples/shared_04_calloc examples/shared_05_threads
	rm -f examples/static_01_basic examples/static_02_array examples/static_03_struct examples/static_04_large examples/static_05_threads

test: all examples
	$(CC) $(CFLAGS) -o tests/test_basic tests/test_basic.c -L. -lhmalloc -Wl,-rpath,.
	$(CC) $(CFLAGS) -o tests/test_threads tests/test_threads.c -L. -lhmalloc -Wl,-rpath,.
	./tests/test_basic
	./tests/test_threads

.PHONY: all clean test examples
