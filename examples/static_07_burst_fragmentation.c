#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 4
#define ITERS 50
#define BATCH_SIZE 5000

void *worker_std(void *arg) {
    (void)arg;
    void *ptrs[BATCH_SIZE];
    for (int it = 0; it < ITERS; it++) {
        for (int i = 0; i < BATCH_SIZE; i++) {
            size_t sz = (i % 3 == 0) ? 64 : ((i % 3 == 1) ? 1024 : 8192);
            ptrs[i] = malloc(sz);
            *(volatile char *)ptrs[i] = 1;
        }
        for (int i = 0; i < BATCH_SIZE; i += 2) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
        for (int i = 1; i < BATCH_SIZE; i += 2) {
            free(ptrs[i]);
        }
    }
    return NULL;
}

void *worker_hmalloc(void *arg) {
    (void)arg;
    void *ptrs[BATCH_SIZE];
    for (int it = 0; it < ITERS; it++) {
        for (int i = 0; i < BATCH_SIZE; i++) {
            size_t sz = (i % 3 == 0) ? 64 : ((i % 3 == 1) ? 1024 : 8192);
            ptrs[i] = hmalloc(sz);
            *(volatile char *)ptrs[i] = 1;
        }
        for (int i = 0; i < BATCH_SIZE; i += 2) {
            hfree(ptrs[i]);
            ptrs[i] = NULL;
        }
        for (int i = 1; i < BATCH_SIZE; i += 2) {
            hfree(ptrs[i]);
        }
    }
    return NULL;
}

int main() {
    printf("[Static 07] Burst Fragmentation Test\n");
    
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
