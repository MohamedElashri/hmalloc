#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("[Shared 04] Zero-initialized Allocation\n");
    size_t count = 50;
    int *data = calloc(count, sizeof(int));
    if (!data) return 1;
    
    int sum = 0;
    for (size_t i = 0; i < count; i++) sum += data[i];
    
    printf("Sum of calloc'd array (should be 0): %d\n", sum);
    free(data);
    return 0;
}
