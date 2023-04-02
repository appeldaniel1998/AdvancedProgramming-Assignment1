#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node {
    char *var;
    char *value;
    struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;

//display the list
//void printList() {
//    struct node *ptr = head;
//    printf("[ ");
//
//    //start from the beginning
//    while (ptr != NULL) {
//        printf("(%s,%s) ", ptr->var, ptr->value);
//        ptr = ptr->next;
//    }
//
//    printf(" ]\n");
//}

//insert link at the first location
void insertFirst(char *var, char *value) {
    //create a link
    struct node *link = (struct node *) malloc(sizeof(struct node));

    char* tempVar = (char*) malloc(sizeof (var));
    strcpy(tempVar, var);

    char* tempVal = (char*) malloc(sizeof (value));
    strcpy(tempVal, value);


    link->var = tempVar;
    link->value = tempVal;

    if (head == NULL) {
        //point it to old first node
        link->next = NULL;
    } else {
        //point it to old first node
        link->next = head;
    }
    //point first to new first node
    head = link;
}



//is list empty
bool isEmpty() {
    return head == NULL;
}


//find a link with given key
struct node *find(char *var) {

    //start from the first link
    current = head;

    //if list is empty
    if (head == NULL) {
        return NULL;
    }

    //navigate through list
    while (strcmp(current->var, var) != 0) {

        //if it is last node
        if (current->next == NULL) {
            return NULL;
        } else {
            //go to next link
            current = current->next;
        }
    }

    //if data found, return the current Link
    return current;
}
