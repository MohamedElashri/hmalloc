#include <stdlib.h>
#include <stdio.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

int main() {
    printf("[Shared 03] Linked List Allocation\n");
    Node *head = NULL;
    for (int i = 0; i < 5; i++) {
        Node *n = malloc(sizeof(Node));
        n->data = i;
        n->next = head;
        head = n;
    }
    
    Node *curr = head;
    while (curr) {
        printf("Node: %d\n", curr->data);
        Node *temp = curr;
        curr = curr->next;
        free(temp);
    }
    return 0;
}
