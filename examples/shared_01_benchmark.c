#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ALLOC_COUNT 10000000

int main() {
    printf("[Shared 01] Single-Threaded Benchmark\n");
    printf("Allocating and freeing %d blocks of 64 bytes...\n", ALLOC_COUNT);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = malloc(64);
        free(p);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    
    printf("Time taken: %f seconds\n", time_taken);
    printf("Operations per second: %.2f\n", (ALLOC_COUNT * 2) / time_taken);
    return 0;
}
