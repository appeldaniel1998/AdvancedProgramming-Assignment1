#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the structure for the list node
typedef struct stringNode {
    char* data;
    struct stringNode* prev;
    struct stringNode* next;
} node_t;

// Function to create a new node
node_t* create_node(char* data) {
    node_t* new_node = (node_t*) malloc(sizeof(node_t));
    new_node->data = (char*) malloc(strlen(data) + 1);
    strcpy(new_node->data, data);
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

// Function to insert a new node at the beginning of the list
void insert_beginning(node_t** head_ref, char* data) {
    node_t* new_node = create_node(data);
    new_node->next = *head_ref;
    if (*head_ref != NULL) {
        (*head_ref)->prev = new_node;
    }
    *head_ref = new_node;
}

// Function to insert a new node at the end of the list
void insert_end(node_t** head_ref, char* data) {
    node_t* new_node = create_node(data);
    node_t* currentNode = *head_ref;
    if (currentNode == NULL) {
        *head_ref = new_node;
        return;
    }
    while (currentNode->next != NULL) {
        currentNode = currentNode->next;
    }
    currentNode->next = new_node;
    new_node->prev = currentNode;
}

// Function to remove a node from the list
void remove_node(node_t** head_ref, node_t* node_to_remove) {
    if (*head_ref == NULL || node_to_remove == NULL) {
        return;
    }
    if (*head_ref == node_to_remove) {
        *head_ref = node_to_remove->next;
    }
    if (node_to_remove->next != NULL) {
        node_to_remove->next->prev = node_to_remove->prev;
    }
    if (node_to_remove->prev != NULL) {
        node_to_remove->prev->next = node_to_remove->next;
    }
    free(node_to_remove->data);
    free(node_to_remove);
}

// Function to get the next node in the list
node_t* getNext(node_t* currentNode) {
    return currentNode->next;
}

// Function to get the previous node in the list
node_t* getPrev(node_t* currentNode) {
    return currentNode->prev;
}

// Function to print the list
void print_list(node_t* headNode) {
    while (headNode != NULL) {
        printf("%s ", headNode->data);
        headNode = headNode->next;
    }
    printf("\n");
}

// Function to get the item at a specific index (counting from 0 as the head)
char* get_item_at_index(node_t* firstHead, int index) {
    node_t* currentHead = firstHead;
    int i = 0;
    while (currentHead != NULL && i < index) {
        currentHead = currentHead->next;
        i++;
    }
    if (currentHead == NULL) {
        return NULL;
    } else {
        return currentHead->data;
    }
}