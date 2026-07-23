#include "hmalloc.h"
#include "internal.h"
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

const uint32_t class_to_size[NUM_SIZE_CLASSES] = {
    16, 32, 48, 64, 80, 96, 112, 128,
    160, 192, 224, 256,
    384, 512, 768, 1024,
    1536, 2048, 3072, 4096,
    8192, 16384, 32768
};

static pthread_mutex_t arena_lock = PTHREAD_MUTEX_INITIALIZER;
static Slab *partial_slabs[NUM_SIZE_CLASSES] = {0};

__thread Tcache tcache = {0};

int size_to_class(size_t size) {
    if (size <= 128) {
        return (size == 0) ? 0 : ((size - 1) / 16);
    }
    for (int i = 8; i < NUM_SIZE_CLASSES; i++) {
        if (size <= class_to_size[i]) return i;
    }
    return -1;
}

void *os_alloc(size_t size) {
    size_t alloc_size = size + SLAB_ALIGNMENT;
    void *ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return NULL;

    uintptr_t uptr = (uintptr_t)ptr;
    uintptr_t aligned = (uptr + SLAB_ALIGNMENT - 1) & SLAB_MASK;
    
    size_t pre_padding = aligned - uptr;
    if (pre_padding > 0) {
        munmap((void *)uptr, pre_padding);
    }
    size_t post_padding = alloc_size - pre_padding - size;
    if (post_padding > 0) {
        munmap((void *)(aligned + size), post_padding);
    }

    return (void *)aligned;
}

void os_free(void *ptr, size_t size) {
    munmap(ptr, size);
}

static void *refill_tcache(int cls) {
    uint32_t bsize = class_to_size[cls];
    pthread_mutex_lock(&arena_lock);
    
    Slab *s = partial_slabs[cls];
    if (s == NULL) {
        s = os_alloc(SLAB_ALIGNMENT);
        if (!s) {
            pthread_mutex_unlock(&arena_lock);
            return NULL;
        }
        s->magic = SLAB_MAGIC;
        s->size_class = cls;
        s->block_size = bsize;
        
        char *start = (char *)s + sizeof(Slab);
        uintptr_t ustart = (uintptr_t)start;
        ustart = (ustart + 15) & ~15;
        start = (char *)ustart;

        uint32_t num_blocks = (SLAB_ALIGNMENT - (start - (char*)s)) / bsize;
        s->free_count = num_blocks;
        s->next = NULL;
        
        char *curr = start;
        for (uint32_t i = 0; i < num_blocks - 1; i++) {
            *(void **)curr = curr + bsize;
            curr += bsize;
        }
        *(void **)curr = NULL;
        s->free_list = start;
        
        partial_slabs[cls] = s;
    }
    
    int batch = s->free_count / 2;
    if (batch == 0) batch = 1;
    if (batch > 32) batch = 32;

    void *first = s->free_list;
    void *last = first;
    for (int i = 1; i < batch; i++) {
        last = *(void **)last;
    }
    
    s->free_list = *(void **)last;
    s->free_count -= batch;
    
    if (s->free_count == 0) {
        partial_slabs[cls] = s->next;
    }
    
    pthread_mutex_unlock(&arena_lock);
    
    if (batch > 1) {
        *(void **)last = tcache.free_list[cls];
        tcache.free_list[cls] = *(void **)first;
    }
    
    return first;
}

void *hmalloc(size_t size) {
    if (size == 0) return NULL;
    
    if (size <= MAX_SMALL_SIZE) {
        int cls = size_to_class(size);
        if (cls >= 0) {
            void *ptr = tcache.free_list[cls];
            if (ptr) {
                tcache.free_list[cls] = *(void **)ptr;
                return ptr;
            }
            return refill_tcache(cls);
        }
    }
    
    size_t alloc_size = size + sizeof(LargeBlock);
    LargeBlock *lb = os_alloc(alloc_size);
    if (!lb) return NULL;
    lb->magic = LARGE_MAGIC;
    lb->size = alloc_size;
    
    return (void *)(lb + 1);
}

void hfree(void *ptr) {
    if (!ptr) return;
    
    uintptr_t uptr = (uintptr_t)ptr;
    void *header_ptr = (void *)(uptr & SLAB_MASK);
    
    uint32_t magic = *(uint32_t *)header_ptr;
    
    if (magic == SLAB_MAGIC) {
        Slab *s = (Slab *)header_ptr;
        int cls = s->size_class;
        
        *(void **)ptr = tcache.free_list[cls];
        tcache.free_list[cls] = ptr;
    } else if (magic == LARGE_MAGIC) {
        LargeBlock *lb = (LargeBlock *)header_ptr;
        size_t size = lb->size;
        os_free(header_ptr, size);
    }
}

void *hcalloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    if (nmemb != 0 && total / nmemb != size) return NULL;
    
    void *ptr = hmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *hrealloc(void *ptr, size_t size) {
    if (!ptr) return hmalloc(size);
    if (size == 0) {
        hfree(ptr);
        return NULL;
    }
    
    uintptr_t uptr = (uintptr_t)ptr;
    void *header_ptr = (void *)(uptr & SLAB_MASK);
    uint32_t magic = *(uint32_t *)header_ptr;
    
    size_t old_size = 0;
    if (magic == SLAB_MAGIC) {
        Slab *s = (Slab *)header_ptr;
        old_size = s->block_size;
    } else if (magic == LARGE_MAGIC) {
        LargeBlock *lb = (LargeBlock *)header_ptr;
        old_size = lb->size - sizeof(LargeBlock);
    } else {
        return NULL;
    }
    
    if (old_size >= size) return ptr;
    
    void *new_ptr = hmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        hfree(ptr);
    }
    return new_ptr;
}

#ifdef __GNUC__
void *malloc(size_t size) __attribute__((alias("hmalloc")));
void free(void *ptr) __attribute__((alias("hfree")));
void *calloc(size_t nmemb, size_t size) __attribute__((alias("hcalloc")));
void *realloc(void *ptr, size_t size) __attribute__((alias("hrealloc")));
#endif
