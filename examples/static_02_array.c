#include "../include/hmalloc.h"
#include <stdio.h>

int main() {
    printf("[Static 02] Dynamic Array using hrealloc\n");
    int *arr = (int*)hmalloc(5 * sizeof(int));
    for (int i = 0; i < 5; i++) arr[i] = i * i;
    
    arr = (int*)hrealloc(arr, 10 * sizeof(int));
    for (int i = 5; i < 10; i++) arr[i] = i * i;
    
    for (int i = 0; i < 10; i++) printf("%d ", arr[i]);
    printf("\n");
    hfree(arr);
    return 0;
}
