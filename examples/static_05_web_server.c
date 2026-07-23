#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define NUM_REQUESTS 100000

void *handle_std(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_REQUESTS; i++) {
        void *req = malloc(64);
        void *headers = malloc(1024);
        void *body = malloc(4096);
        *(volatile char *)req = 1;
        *(volatile char *)headers = 1;
        *(volatile char *)body = 1;
        free(body);
        free(headers);
        free(req);
    }
    return NULL;
}

void *handle_hmalloc(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_REQUESTS; i++) {
        void *req = hmalloc(64);
        void *headers = hmalloc(1024);
        void *body = hmalloc(4096);
        *(volatile char *)req = 1;
        *(volatile char *)headers = 1;
        *(volatile char *)body = 1;
        hfree(body);
        hfree(headers);
        hfree(req);
    }
    return NULL;
}

int main() {
    printf("[Static 05] Web Server Request Simulation Comparison\n");
    
    pthread_t threads[4];
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 4; i++) pthread_create(&threads[i], NULL, handle_std, NULL);
    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_std = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 4; i++) pthread_create(&threads[i], NULL, handle_hmalloc, NULL);
    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_hmalloc = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    
    printf("Standard malloc time: %f seconds\n", t_std);
    printf("hmalloc time:         %f seconds\n", t_hmalloc);
    if (t_hmalloc > 0) printf("Speedup: %.2fx\n", t_std / t_hmalloc);
    
    return 0;
}
