# hmalloc

An optimized, thread-aware memory allocator in C utilizing thread-local caching and segregated size classes.

## Build

To compile the shared library (`libhmalloc.so`) and run tests:

```bash
make
make test
```

## Usage

You can use `hmalloc` as a drop-in replacement for the standard system allocator via `LD_PRELOAD`:

```bash
LD_PRELOAD=./libhmalloc.so <your_command>
```

Example:
```bash
LD_PRELOAD=./libhmalloc.so ls -la
```

To link it statically into your own C program:
1. Include `include/hmalloc.h`.
2. Compile and link with `-L. -lhmalloc`.
