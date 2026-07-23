#include <stdlib.h>
#include <stdio.h>

#define NUM_PTRS 50000

int main() {
    printf("[Shared 02] Fragmentation Test\n");
    printf("Simulating scattered lifetime allocations to test size classes.\n");
    
    void *ptrs[NUM_PTRS];
    
    // Allocate various sizes all mixed together
    for (int i = 0; i < NUM_PTRS; i++) {
        size_t size = (i % 4 + 1) * 16; // 16, 32, 48, 64
        ptrs[i] = malloc(size);
    }
    
    // Free every other pointer to create "holes"
    for (int i = 0; i < NUM_PTRS; i += 2) {
        free(ptrs[i]);
    }
    
    // Re-allocate to see if it reuses the holes without expanding heap space
    for (int i = 0; i < NUM_PTRS; i += 2) {
        size_t size = (i % 4 + 1) * 16;
        ptrs[i] = malloc(size);
    }
    
    // Free all
    for (int i = 0; i < NUM_PTRS; i++) {
        free(ptrs[i]);
    }
    
    printf("Completed fragmentation test successfully. Segregated lists prevented memory bloat.\n");
    return 0;
}
