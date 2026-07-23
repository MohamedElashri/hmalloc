#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("[Shared 02] Dynamic Array Resizing\n");
    int *arr = malloc(5 * sizeof(int));
    for (int i = 0; i < 5; i++) arr[i] = i * i;
    
    arr = realloc(arr, 10 * sizeof(int));
    for (int i = 5; i < 10; i++) arr[i] = i * i;
    
    for (int i = 0; i < 10; i++) printf("%d ", arr[i]);
    printf("\n");
    free(arr);
    return 0;
}
