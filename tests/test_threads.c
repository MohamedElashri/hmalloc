#include "hmalloc.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define ALLOCS_PER_THREAD 10000

void *thread_func(void *arg) {
    (void)arg;
    void *ptrs[ALLOCS_PER_THREAD];
    
    unsigned int seed = (unsigned int)pthread_self();
    for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
        size_t size = (rand_r(&seed) % 4096) + 1;
        ptrs[i] = hmalloc(size);
        assert(ptrs[i] != NULL);
        
        char *c = (char *)ptrs[i];
        c[0] = 'a';
        c[size - 1] = 'b';
    }
    
    for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
        hfree(ptrs[i]);
    }
    
    return NULL;
}

int main() {
    printf("Running thread tests...\n");
    pthread_t threads[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Thread tests passed.\n");
    return 0;
}
