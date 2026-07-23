#ifndef HMALLOC_INTERNAL_H
#define HMALLOC_INTERNAL_H

#include <stdint.h>
#include <pthread.h>
#include <stddef.h>

/* Use 256KB alignment for all large OS allocations to easily find metadata */
#define SLAB_ALIGNMENT (256 * 1024)
#define SLAB_MASK (~((uintptr_t)SLAB_ALIGNMENT - 1))

#define SLAB_MAGIC 0x51AB51AB
#define LARGE_MAGIC 0x1A6E1A6E

#define MAX_SMALL_SIZE 32768
#define NUM_SIZE_CLASSES 24

typedef struct Slab {
    uint32_t magic;
    uint32_t size_class;
    uint32_t block_size;
    uint32_t free_count;
    struct Slab *next;
    void *free_list;
} Slab;

typedef struct {
    uint32_t magic;
    size_t size; /* Total size passed to mmap, including alignment overhead */
} LargeBlock;

typedef struct {
    void *free_list[NUM_SIZE_CLASSES];
} Tcache;

extern const uint32_t class_to_size[NUM_SIZE_CLASSES];

/* Internal functions */
void *os_alloc(size_t size);
void os_free(void *ptr, size_t size);
int size_to_class(size_t size);

#endif /* HMALLOC_INTERNAL_H */
