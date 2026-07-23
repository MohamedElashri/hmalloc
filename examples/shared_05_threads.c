#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void *worker(void *arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 1000; i++) {
        void *p = malloc( (id % 128) + 16 );
        free(p);
    }
    printf("Thread %d finished\n", id);
    return NULL;
}

int main() {
    printf("[Shared 05] Multithreaded Allocation\n");
    pthread_t threads[4];
    int ids[4] = {1, 2, 3, 4};
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
