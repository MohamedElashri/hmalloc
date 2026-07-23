#include "hmalloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    printf("Running basic tests...\n");
    
    void *p1 = hmalloc(16);
    assert(p1 != NULL);
    strcpy(p1, "hello");
    assert(strcmp(p1, "hello") == 0);
    hfree(p1);
    
    void *p2 = hmalloc(65536);
    assert(p2 != NULL);
    memset(p2, 'A', 65536);
    hfree(p2);
    
    int *p3 = hcalloc(10, sizeof(int));
    assert(p3 != NULL);
    for (int i = 0; i < 10; i++) {
        assert(p3[i] == 0);
    }
    hfree(p3);
    
    char *p4 = hmalloc(10);
    strcpy(p4, "test");
    p4 = hrealloc(p4, 100);
    assert(strcmp(p4, "test") == 0);
    hfree(p4);
    
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = hmalloc(32);
        assert(ptrs[i] != NULL);
    }
    for (int i = 0; i < 100; i++) {
        hfree(ptrs[i]);
    }
    
    printf("Basic tests passed.\n");
    return 0;
}
