#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    printf("[Shared 01] Basic Allocation\n");
    char *str = malloc(100);
    if (!str) return 1;
    strcpy(str, "Hello World from hmalloc via LD_PRELOAD!");
    printf("%s\n", str);
    free(str);
    return 0;
}
