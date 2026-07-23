#define _GNU_SOURCE
#include "../include/hmalloc.h"
#include "internal.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

const uint32_t class_to_size[NUM_SIZE_CLASSES] = {
    16, 32, 48, 64, 80, 96, 112, 128, 160, 192, 224, 256,
    320, 384, 448, 512, 1024, 2048, 4096, 8192, 12288, 16384, 24576, 32768
};

Arena arenas[NUM_ARENAS];
uintptr_t secret_key = 0;
pthread_key_t tcache_key;
bool system_initialized = false;
__thread Tcache tcache = {0};

void pre_fork() {
    for (int i = 0; i < NUM_ARENAS; i++) pthread_mutex_lock(&arenas[i].lock);
}
void post_fork() {
    for (int i = 0; i < NUM_ARENAS; i++) pthread_mutex_unlock(&arenas[i].lock);
}

void tcache_destroy(void *arg) {
    (void)arg;
    if (!tcache.initialized) return;
    
    int a_id = tcache.arena_id;
    pthread_mutex_lock(&arenas[a_id].lock);
    
    for (int c = 0; c < NUM_SIZE_CLASSES; c++) {
        void *ptr = tcache.free_list[c];
        while (ptr) {
            void *next = deobfuscate_ptr(*(void **)ptr);
            Slab *s = (Slab *)((uintptr_t)ptr & SLAB_MASK);
            
            *(void **)ptr = s->free_list;
            s->free_list = ptr;
            s->free_count++;
            
            if (s->free_count == s->total_blocks) {
                if (s->prev) s->prev->next = s->next;
                else arenas[a_id].partial_slabs[s->size_class] = s->next;
                if (s->next) s->next->prev = s->prev;
                
                arenas[a_id].active_slabs--;
                os_free(s, SLAB_ALIGNMENT, false);
            } else if (s->free_count == 1) {
                s->next = arenas[a_id].partial_slabs[s->size_class];
                s->prev = NULL;
                if (s->next) s->next->prev = s;
                arenas[a_id].partial_slabs[s->size_class] = s;
            }
            
            ptr = next;
        }
        tcache.free_list[c] = NULL;
    }
    pthread_mutex_unlock(&arenas[a_id].lock);
    tcache.initialized = false;
}

__attribute__((constructor)) void hmalloc_global_init() {
    if (system_initialized) return;
    for (int i = 0; i < NUM_ARENAS; i++) {
        pthread_mutex_init(&arenas[i].lock, NULL);
        for (int j = 0; j < NUM_SIZE_CLASSES; j++) {
            arenas[i].partial_slabs[j] = NULL;
        }
        arenas[i].active_slabs = 0;
    }
    
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        read(fd, &secret_key, sizeof(secret_key));
        close(fd);
    } else {
        secret_key = 0xDEADBEEFCAFEBABE;
    }
    
    pthread_key_create(&tcache_key, tcache_destroy);
    pthread_atfork(pre_fork, post_fork, post_fork);
    system_initialized = true;
}

void init_thread() {
    if (tcache.initialized) return;
    if (!system_initialized) hmalloc_global_init();
    tcache.arena_id = ((uintptr_t)pthread_self() >> 12) % NUM_ARENAS;
    pthread_setspecific(tcache_key, (void*)1);
    tcache.initialized = true;
}

void *os_alloc(size_t size, bool guard_pages) {
    size_t actual_size = size;
    size_t page_size = sysconf(_SC_PAGESIZE);
#ifdef SECURITY_HARDENING
    if (guard_pages) actual_size += 2 * page_size;
#endif
    
    void *ptr = mmap(NULL, actual_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return NULL;
    
#ifdef SECURITY_HARDENING
    if (guard_pages) {
        mprotect(ptr, page_size, PROT_NONE);
        mprotect((char*)ptr + actual_size - page_size, page_size, PROT_NONE);
        ptr = (char*)ptr + page_size;
    }
#endif
    return ptr;
}

void os_free(void *ptr, size_t size, bool guard_pages) {
    size_t actual_size = size;
#ifdef SECURITY_HARDENING
    if (guard_pages) {
        size_t page_size = sysconf(_SC_PAGESIZE);
        ptr = (char*)ptr - page_size;
        actual_size += 2 * page_size;
    }
#endif
    munmap(ptr, actual_size);
}

static void *allocate_aligned_os_memory(size_t size) {
    void *raw = mmap(NULL, size + SLAB_ALIGNMENT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw == MAP_FAILED) return NULL;
    
    uintptr_t raw_addr = (uintptr_t)raw;
    uintptr_t aligned_addr = (raw_addr + SLAB_ALIGNMENT - 1) & SLAB_MASK;
    size_t offset = aligned_addr - raw_addr;
    
    if (offset > 0) munmap(raw, offset);
    size_t tail = (size + SLAB_ALIGNMENT) - offset - size;
    if (tail > 0) munmap((void*)(aligned_addr + size), tail);
    
    return (void *)aligned_addr;
}

static const uint8_t size_class_lookup[33] = {
    0, 0, 1, 2, 3, 4, 5, 6, 7,
    8, 8, 9, 9, 10, 10, 11, 11,
    12, 12, 12, 12, 13, 13, 13, 13,
    14, 14, 14, 14, 15, 15, 15, 15
};

int size_to_class(size_t size) {
    if (__builtin_expect(size <= 512, 1)) {
        return size_class_lookup[(size + 15) / 16];
    }
    for (int i = 16; i < NUM_SIZE_CLASSES; i++) {
        if (size <= class_to_size[i]) return i;
    }
    return -1;
}

static Slab *allocate_slab(int c) {
    size_t block_size = class_to_size[c];
    
    // Ensure slab is 256KB aligned
    void *aligned_addr = allocate_aligned_os_memory(SLAB_ALIGNMENT);
    if (!aligned_addr) return NULL;
    
    Slab *s = (Slab *)aligned_addr;
    s->magic = SLAB_MAGIC;
    s->size_class = c;
    s->block_size = block_size;
    
    size_t header_size = sizeof(Slab);
    size_t usable_space = SLAB_ALIGNMENT - header_size;
    s->total_blocks = usable_space / block_size;
    s->free_count = s->total_blocks;
    
    char *curr = (char *)s + header_size;
    s->free_list = curr;
    for (uint32_t i = 0; i < s->total_blocks - 1; i++) {
        *(void **)curr = (curr + block_size);
        curr += block_size;
    }
    *(void **)curr = NULL;
    
    return s;
}

static void refill_tcache(int c) {
    int a_id = tcache.arena_id;
    pthread_mutex_lock(&arenas[a_id].lock);
    
    Slab *s = arenas[a_id].partial_slabs[c];
    if (!s) {
        s = allocate_slab(c);
        if (!s) {
            pthread_mutex_unlock(&arenas[a_id].lock);
            return;
        }
        arenas[a_id].active_slabs++;
        s->next = NULL;
        s->prev = NULL;
        arenas[a_id].partial_slabs[c] = s;
    }
    
    uint32_t blocks_to_move = s->free_count > 32 ? 32 : s->free_count;
    void *curr = s->free_list;
    void *tail = curr;
    
    for (uint32_t i = 0; i < blocks_to_move - 1; i++) {
        tail = *(void **)tail;
    }
    
    s->free_list = *(void **)tail;
    s->free_count -= blocks_to_move;
    
    if (s->free_count == 0) {
        arenas[a_id].partial_slabs[c] = s->next;
        if (s->next) s->next->prev = NULL;
    }
    
    pthread_mutex_unlock(&arenas[a_id].lock);
    
    // Obfuscate the transferred list
    void *list_curr = curr;
    for (uint32_t i = 0; i < blocks_to_move - 1; i++) {
        void *next_ptr = *(void **)list_curr;
        *(void **)list_curr = obfuscate_ptr(next_ptr);
        list_curr = next_ptr;
    }
    *(void **)list_curr = obfuscate_ptr(NULL);
    
    tcache.free_list[c] = curr;
}

void *hmalloc(size_t size) {
    if (__builtin_expect(size == 0, 0)) return NULL;
    
    if (__builtin_expect(size <= MAX_SMALL_SIZE, 1)) {
        if (__builtin_expect(!tcache.initialized, 0)) init_thread();
        int c = size_to_class(size);
        if (__builtin_expect(!tcache.free_list[c], 0)) {
            refill_tcache(c);
            if (!tcache.free_list[c]) return NULL;
        }
        
        void *ptr = tcache.free_list[c];
        tcache.free_list[c] = deobfuscate_ptr(*(void **)ptr);
        return ptr;
    } else {
        size_t total_size = size + sizeof(LargeBlock);
#ifdef SECURITY_HARDENING
        size_t page_size = sysconf(_SC_PAGESIZE);
        // Page align the total size for mprotect boundaries
        total_size = (total_size + page_size * 2 + page_size - 1) & ~(page_size - 1);
#endif
        void *raw = allocate_aligned_os_memory(total_size);
        if (!raw) return NULL;
        
        LargeBlock *lb = (LargeBlock *)raw;
        lb->magic = LARGE_MAGIC;
        lb->size = total_size;
        
        void *user_ptr = (char *)raw + sizeof(LargeBlock);
        
#ifdef SECURITY_HARDENING
        // Guard front (after LargeBlock header)
        void *front_guard = (char *)raw + page_size;
        mprotect(front_guard, page_size, PROT_NONE);
        user_ptr = (char *)front_guard + page_size;
        
        // Guard back
        void *back_guard = (char *)user_ptr + ((size + page_size - 1) & ~(page_size - 1));
        if ((char *)back_guard + page_size <= (char *)raw + total_size) {
            mprotect(back_guard, page_size, PROT_NONE);
        }
#endif
        
        return user_ptr;
    }
}

void hfree(void *ptr) {
    if (__builtin_expect(!ptr, 0)) return;
    
    uintptr_t uptr = (uintptr_t)ptr;
    Slab *s = (Slab *)(uptr & SLAB_MASK);
    
    if (__builtin_expect(s->magic == SLAB_MAGIC, 1)) {
        int c = s->size_class;
        if (__builtin_expect(tcache.initialized, 1)) {
            *(void **)ptr = obfuscate_ptr(tcache.free_list[c]);
            tcache.free_list[c] = ptr;
        } else {
            int a_id = ((uintptr_t)pthread_self() >> 12) % NUM_ARENAS;
            pthread_mutex_lock(&arenas[a_id].lock);
            *(void **)ptr = s->free_list;
            s->free_list = ptr;
            s->free_count++;
            
            if (s->free_count == s->total_blocks) {
                if (s->prev) s->prev->next = s->next;
                else arenas[a_id].partial_slabs[c] = s->next;
                if (s->next) s->next->prev = s->prev;
                arenas[a_id].active_slabs--;
                munmap(s, SLAB_ALIGNMENT);
            } else if (s->free_count == 1) {
                s->next = arenas[a_id].partial_slabs[c];
                s->prev = NULL;
                if (s->next) s->next->prev = s;
                arenas[a_id].partial_slabs[c] = s;
            }
            pthread_mutex_unlock(&arenas[a_id].lock);
        }
    } else if (__builtin_expect(s->magic == LARGE_MAGIC, 0)) {
        LargeBlock *lb = (LargeBlock *)s;
        munmap(lb, lb->size);
    }
}

void *hcalloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = hmalloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void *hrealloc(void *ptr, size_t size) {
    if (!ptr) return hmalloc(size);
    if (size == 0) { hfree(ptr); return NULL; }
    
    size_t old_size = 0;
    
    uintptr_t uptr = (uintptr_t)ptr;
    Slab *s = (Slab *)(uptr & SLAB_MASK);
    
    if (s->magic == SLAB_MAGIC) {
        old_size = s->block_size;
    } else if (s->magic == LARGE_MAGIC) {
        LargeBlock *lb = (LargeBlock *)s;
#ifdef SECURITY_HARDENING
        size_t page_size = sysconf(_SC_PAGESIZE);
        old_size = lb->size - sizeof(LargeBlock) - page_size * 2;
#else
        old_size = lb->size - sizeof(LargeBlock);
#endif
    } else {
        return NULL; 
    }
    
    if (size <= old_size) return ptr;
    
    void *new_ptr = hmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        hfree(ptr);
    }
    return new_ptr;
}

void *hmemalign(size_t alignment, size_t size) {
    if ((alignment & (alignment - 1)) != 0) return NULL; 
    
    if (alignment <= 16) return hmalloc(size);
    
    if (alignment <= (size_t)sysconf(_SC_PAGESIZE)) {
        size_t actual_size = size > MAX_SMALL_SIZE ? size : MAX_SMALL_SIZE + 1;
        return hmalloc(actual_size);
    }
    return NULL; // TODO: Implement exact extreme alignments
}

int hposix_memalign(void **memptr, size_t alignment, size_t size) {
    if (alignment % sizeof(void *) != 0 || (alignment & (alignment - 1)) != 0) return 22;
    void *ptr = hmemalign(alignment, size);
    if (!ptr) return 12;
    *memptr = ptr;
    return 0;
}

void *haligned_alloc(size_t alignment, size_t size) { return hmemalign(alignment, size); }
void *hvalloc(size_t size) { return hmemalign(sysconf(_SC_PAGESIZE), size); }

/* Aliases for LD_PRELOAD override */
void *malloc(size_t size) __attribute__((alias("hmalloc")));
void free(void *ptr) __attribute__((alias("hfree")));
void *calloc(size_t nmemb, size_t size) __attribute__((alias("hcalloc")));
void *realloc(void *ptr, size_t size) __attribute__((alias("hrealloc")));
int posix_memalign(void **memptr, size_t alignment, size_t size) __attribute__((alias("hposix_memalign")));
void *memalign(size_t alignment, size_t size) __attribute__((alias("hmemalign")));
void *aligned_alloc(size_t alignment, size_t size) __attribute__((alias("haligned_alloc")));
void *valloc(size_t size) __attribute__((alias("hvalloc")));
