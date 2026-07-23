#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 8
#define ALLOC_COUNT 1000000

void *worker_std(void *arg) {
    (void)arg;
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = malloc(64);
        *(volatile char *)p = 1;
        free(p);
    }
    return NULL;
}

void *worker_hmalloc(void *arg) {
    (void)arg;
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = hmalloc(64);
        *(volatile char *)p = 1;
        hfree(p);
    }
    return NULL;
}

int main() {
    printf("[Static 03] Thread-Local Cache (Tcache) Scale Comparison\n");
    printf("Spawning %d threads, each doing %d allocations...\n", NUM_THREADS, ALLOC_COUNT);
    
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
