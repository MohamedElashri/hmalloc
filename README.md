# hmalloc

An optimized, thread-aware memory allocator in C utilizing thread-local caching and segregated size classes.

## Build

To compile the shared library (`libhmalloc.so`), static library (`libhmalloc.a`), and run tests:

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

## Examples

We provide 10 code examples in the `examples/` directory demonstrating different levels of complexity with both linkage formats:

```bash
make examples
```

**1. Shared Library (LD_PRELOAD)**:
These examples use the standard `malloc`, `free`, and `calloc` functions from `<stdlib.h>`. When run with `LD_PRELOAD`, our `libhmalloc.so` intercepts them.

- `examples/shared_01_basic.c` - Basic allocation
- `examples/shared_02_array.c` - Dynamic array resizing via `realloc`
- `examples/shared_03_struct.c` - Linked list node allocation
- `examples/shared_04_calloc.c` - Zero-initialized chunk allocation
- `examples/shared_05_threads.c` - Multi-threaded memory contention

Run them like this:
```bash
LD_PRELOAD=./libhmalloc.so ./examples/shared_01_basic
```

**2. Static Library**:
These examples explicitly include `hmalloc.h` and use the `hmalloc` API directly. They are linked statically against `libhmalloc.a`.

- `examples/static_01_basic.c` - Basic allocation
- `examples/static_02_array.c` - Dynamic array resizing via `hrealloc`
- `examples/static_03_struct.c` - Linked list node allocation
- `examples/static_04_large.c` - Large allocation triggering `mmap` directly
- `examples/static_05_threads.c` - Multi-threaded memory contention

Run them like this:
```bash
./examples/static_01_basic
```
