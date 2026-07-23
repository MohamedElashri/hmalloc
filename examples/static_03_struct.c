#include "../include/hmalloc.h"
#include <stdio.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

int main() {
    printf("[Static 03] Linked List using hmalloc\n");
    Node *head = NULL;
    for (int i = 0; i < 5; i++) {
        Node *n = (Node*)hmalloc(sizeof(Node));
        n->data = i;
        n->next = head;
        head = n;
    }
    
    Node *curr = head;
    while (curr) {
        printf("Node: %d\n", curr->data);
        Node *temp = curr;
        curr = curr->next;
        hfree(temp);
    }
    return 0;
}
