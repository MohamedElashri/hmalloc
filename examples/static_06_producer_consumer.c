#include "../include/hmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define ALLOC_COUNT 1000000
#define BATCH_SIZE 1000

void *shared_queue[ALLOC_COUNT];
int produce_idx = 0;
int consume_idx = 0;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t q_cond = PTHREAD_COND_INITIALIZER;
int done_producing = 0;

void *producer_std(void *arg) {
    (void)arg;
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = malloc(64);
        *(volatile char *)p = 1;
        pthread_mutex_lock(&q_lock);
        shared_queue[produce_idx++] = p;
        pthread_cond_signal(&q_cond);
        pthread_mutex_unlock(&q_lock);
    }
    pthread_mutex_lock(&q_lock);
    done_producing = 1;
    pthread_cond_signal(&q_cond);
    pthread_mutex_unlock(&q_lock);
    return NULL;
}

void *consumer_std(void *arg) {
    (void)arg;
    while (1) {
        void *p = NULL;
        pthread_mutex_lock(&q_lock);
        while (consume_idx >= produce_idx && !done_producing) {
            pthread_cond_wait(&q_cond, &q_lock);
        }
        if (consume_idx < produce_idx) {
            p = shared_queue[consume_idx++];
        } else if (done_producing) {
            pthread_mutex_unlock(&q_lock);
            break;
        }
        pthread_mutex_unlock(&q_lock);
        if (p) free(p);
    }
    return NULL;
}

void *producer_hmalloc(void *arg) {
    (void)arg;
    for (int i = 0; i < ALLOC_COUNT; i++) {
        void *p = hmalloc(64);
        *(volatile char *)p = 1;
        pthread_mutex_lock(&q_lock);
        shared_queue[produce_idx++] = p;
        pthread_cond_signal(&q_cond);
        pthread_mutex_unlock(&q_lock);
    }
    pthread_mutex_lock(&q_lock);
    done_producing = 1;
    pthread_cond_signal(&q_cond);
    pthread_mutex_unlock(&q_lock);
    return NULL;
}

void *consumer_hmalloc(void *arg) {
    (void)arg;
    while (1) {
        void *p = NULL;
        pthread_mutex_lock(&q_lock);
        while (consume_idx >= produce_idx && !done_producing) {
            pthread_cond_wait(&q_cond, &q_lock);
        }
        if (consume_idx < produce_idx) {
            p = shared_queue[consume_idx++];
        } else if (done_producing) {
            pthread_mutex_unlock(&q_lock);
            break;
        }
        pthread_mutex_unlock(&q_lock);
        if (p) hfree(p);
    }
    return NULL;
}

void reset_queue() {
    produce_idx = 0;
    consume_idx = 0;
    done_producing = 0;
}

int main() {
    printf("[Static 06] Producer-Consumer Unbounded Growth Test\n");
    
    pthread_t prod, cons;
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_create(&prod, NULL, producer_std, NULL);
    pthread_create(&cons, NULL, consumer_std, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_std = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    printf("Standard malloc time: %f seconds\n", t_std);
    
    reset_queue();
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_create(&prod, NULL, producer_hmalloc, NULL);
    pthread_create(&cons, NULL, consumer_hmalloc, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_hmalloc = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    printf("hmalloc time:         %f seconds\n", t_hmalloc);
    
    if (t_hmalloc > 0) printf("Speedup: %.2fx\n", t_std / t_hmalloc);
    return 0;
}
