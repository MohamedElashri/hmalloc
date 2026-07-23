#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ITEMS 10000

void *buffer[NUM_ITEMS];
int head = 0;
int tail = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_ITEMS; i++) {
        void *p = malloc(128);
        pthread_mutex_lock(&lock);
        buffer[head++] = p;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_ITEMS; i++) {
        pthread_mutex_lock(&lock);
        while (tail == head) {
            pthread_cond_wait(&cond, &lock);
        }
        void *p = buffer[tail++];
        pthread_mutex_unlock(&lock);
        free(p);
    }
    return NULL;
}

int main() {
    printf("[Shared 03] Cross-Thread Producer-Consumer\n");
    printf("Testing allocations that are freed by different threads (cross-tcache).\n");
    
    pthread_t p, c;
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);
    
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    
    printf("Producer-Consumer test passed.\n");
    return 0;
}
