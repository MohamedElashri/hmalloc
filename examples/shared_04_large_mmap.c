#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    printf("[Shared 04] Large Object Handling\n");
    printf("Allocating objects larger than 32KB to demonstrate direct mmap bypassing.\n");
    
    size_t sizes[] = { 64 * 1024, 256 * 1024, 2 * 1024 * 1024, 10 * 1024 * 1024 };
    
    for (int i = 0; i < 4; i++) {
        printf("Allocating %zu bytes... ", sizes[i]);
        char *buffer = malloc(sizes[i]);
        if (!buffer) {
            printf("FAILED\n");
            return 1;
        }
        
        memset(buffer, 'A', sizes[i]);
        buffer[sizes[i] - 1] = 'Z';
        
        free(buffer);
        printf("SUCCESS\n");
    }
    
    return 0;
}
