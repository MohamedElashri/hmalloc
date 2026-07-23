CC = gcc
SECURITY_FLAGS ?= 
CFLAGS = -Wall -Wextra -O3 -fPIC -pthread -Iinclude $(SECURITY_FLAGS)
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
	$(CC) $(CFLAGS) -o examples/shared_01_benchmark examples/shared_01_benchmark.c
	$(CC) $(CFLAGS) -o examples/shared_02_fragmentation examples/shared_02_fragmentation.c
	$(CC) $(CFLAGS) -o examples/shared_03_producer_consumer examples/shared_03_producer_consumer.c
	$(CC) $(CFLAGS) -o examples/shared_04_large_mmap examples/shared_04_large_mmap.c
	$(CC) $(CFLAGS) -o examples/shared_05_matrix_mul examples/shared_05_matrix_mul.c
	$(CC) $(CFLAGS) -o examples/static_01_benchmark examples/static_01_benchmark.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_02_boundaries examples/static_02_boundaries.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_03_tcache_scale examples/static_03_tcache_scale.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_04_alignment examples/static_04_alignment.c libhmalloc.a
	$(CC) $(CFLAGS) -o examples/static_05_web_server examples/static_05_web_server.c libhmalloc.a

clean:
	rm -f $(OBJS) $(SHARED_TARGET) $(STATIC_TARGET)
	rm -f tests/test_basic tests/test_threads
	rm -f examples/shared_01_benchmark examples/shared_02_fragmentation examples/shared_03_producer_consumer examples/shared_04_large_mmap examples/shared_05_matrix_mul
	rm -f examples/static_01_benchmark examples/static_02_boundaries examples/static_03_tcache_scale examples/static_04_alignment examples/static_05_web_server

test: all examples
	$(CC) $(CFLAGS) -o tests/test_basic tests/test_basic.c -L. -lhmalloc -Wl,-rpath,.
	$(CC) $(CFLAGS) -o tests/test_threads tests/test_threads.c -L. -lhmalloc -Wl,-rpath,.
	./tests/test_basic
	./tests/test_threads

.PHONY: all clean test examples
