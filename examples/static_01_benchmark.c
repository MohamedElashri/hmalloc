#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ALLOC_COUNT 5000000

double measure_standard() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = malloc(64);
        *(volatile char *)p = 1;
        free(p);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
}

double measure_hmalloc() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = hmalloc(64);
        *(volatile char *)p = 1;
        hfree(p);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
}

int main() {
    printf("[Static 01] Native Benchmark Comparison\n");
    printf("Allocating and freeing %d blocks of 64 bytes...\n", ALLOC_COUNT);
    
    double t_std = measure_standard();
    printf("Standard malloc time: %f seconds\n", t_std);
    
    double t_hmalloc = measure_hmalloc();
    printf("hmalloc time:         %f seconds\n", t_hmalloc);
    
    if (t_hmalloc > 0) {
        printf("Speedup: %.2fx\n", t_std / t_hmalloc);
    }
    return 0;
}
