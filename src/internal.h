#ifndef HMALLOC_INTERNAL_H
#define HMALLOC_INTERNAL_H

#include <stdint.h>
#include <pthread.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#define SLAB_ALIGNMENT (256 * 1024)
#define SLAB_MASK (~((uintptr_t)SLAB_ALIGNMENT - 1))

#define SLAB_MAGIC 0x51AB51AB
#define LARGE_MAGIC 0x1A6E1A6E

#define MAX_SMALL_SIZE 32768
#define NUM_SIZE_CLASSES 24
#define NUM_ARENAS 4

typedef struct Slab {
    uint32_t magic;
    uint32_t size_class;
    uint32_t block_size;
    uint32_t free_count;
    uint32_t total_blocks;
    uint32_t arena_id;
    struct Slab *next;
    struct Slab *prev;
    void *free_list;
} Slab;

typedef struct {
    uint32_t magic;
    size_t size; /* Total size passed to mmap */
} LargeBlock;

typedef struct {
    void *free_list[NUM_SIZE_CLASSES];
    uint16_t counts[NUM_SIZE_CLASSES];
    bool initialized;
    int arena_id;
} Tcache;

#define TCACHE_MAX_BLOCKS 256

typedef struct {
    pthread_mutex_t lock;
    Slab *partial_slabs[NUM_SIZE_CLASSES];
    size_t active_slabs;
} Arena;

extern const uint32_t class_to_size[NUM_SIZE_CLASSES];
extern Arena arenas[NUM_ARENAS];
extern uintptr_t secret_key;
extern pthread_key_t tcache_key;
extern bool system_initialized;

/* Internal functions */
void *os_alloc(size_t size, bool guard_pages);
void os_free(void *ptr, size_t size, bool guard_pages);
int size_to_class(size_t size);
void init_thread();
void hmalloc_global_init();

static inline void *obfuscate_ptr(void *ptr) {
#ifdef SECURITY_HARDENING
    return (void *)((uintptr_t)ptr ^ secret_key);
#else
    return ptr;
#endif
}

static inline void *deobfuscate_ptr(void *ptr) {
#ifdef SECURITY_HARDENING
    return (void *)((uintptr_t)ptr ^ secret_key);
#else
    return ptr;
#endif
}

#endif /* HMALLOC_INTERNAL_H */
