#include "../include/hmalloc.h"
#include <stdio.h>

int main() {
    printf("[Static 04] Large Allocation (>32KB)\n");
    size_t size = 1024 * 1024; // 1MB
    char *big_buffer = (char*)hmalloc(size);
    if (!big_buffer) return 1;
    
    big_buffer[0] = 'X';
    big_buffer[size - 1] = 'Y';
    printf("Allocated 1MB successfully. First: %c, Last: %c\n", big_buffer[0], big_buffer[size - 1]);
    
    hfree(big_buffer);
    return 0;
}
