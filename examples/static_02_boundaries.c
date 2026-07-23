#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
    printf("[Static 02] Size Class Boundaries Comparison\n");
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++) {
        void *p = malloc(17);
        *(volatile char *)p = 1;
        free(p);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_std = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000000; i++) {
        void *p = hmalloc(17);
        *(volatile char *)p = 1;
        hfree(p);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_hmalloc = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    printf("Standard malloc 17-byte time: %f seconds\n", t_std);
    printf("hmalloc 17-byte time:         %f seconds\n", t_hmalloc);
    if (t_hmalloc > 0) printf("Speedup: %.2fx\n", t_std / t_hmalloc);
    
    return 0;
}
