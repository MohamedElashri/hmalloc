#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 32
#define ALLOC_COUNT 50000

void *worker_std(void *arg) {
    (void)arg;
    void *ptrs[100];
    for (int i = 0; i < ALLOC_COUNT; i++) {
        int count = (i % 50) + 1;
        for (int j = 0; j < count; j++) {
            size_t sz = ((i + j) % 1024) + 16;
            ptrs[j] = malloc(sz);
            *(volatile char *)ptrs[j] = 1;
        }
        for (int j = 0; j < count; j++) {
            free(ptrs[j]);
        }
    }
    return NULL;
}

void *worker_hmalloc(void *arg) {
    (void)arg;
    void *ptrs[100];
    for (int i = 0; i < ALLOC_COUNT; i++) {
        int count = (i % 50) + 1;
        for (int j = 0; j < count; j++) {
            size_t sz = ((i + j) % 1024) + 16;
            ptrs[j] = hmalloc(sz);
            *(volatile char *)ptrs[j] = 1;
        }
        for (int j = 0; j < count; j++) {
            hfree(ptrs[j]);
        }
    }
    return NULL;
}

int main() {
    printf("[Static 08] Heavy Concurrency Multi-Arena Test\n");
    printf("Spawning %d threads...\n", NUM_THREADS);
    
    pthread_t threads[NUM_THREADS];
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&threads[i], NULL, worker_std, NULL);
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_std = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    printf("Standard malloc time: %f seconds\n", t_std);
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) pthread_create(&threads[i], NULL, worker_hmalloc, NULL);
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_hmalloc = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    printf("hmalloc time:         %f seconds\n", t_hmalloc);
    
    if (t_hmalloc > 0) printf("Speedup: %.2fx\n", t_std / t_hmalloc);
    return 0;
}
