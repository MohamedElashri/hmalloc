#include "../include/hmalloc.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("[Static 01] Basic hmalloc\n");
    char *str = (char*)hmalloc(100);
    if (!str) return 1;
    strcpy(str, "Hello from statically linked hmalloc!");
    printf("%s\n", str);
    hfree(str);
    return 0;
}
