#include <stdio.h>

// test a list

typedef struct node_s {

    node_s *next;
    node_s *prev;
    unsigned int data;
    
} node_t;

typedef struct list_s {
    node_t *head;
    node_t *tail;
    unsigned int items; // number of nodes in list
}

// create list

// create node

// add node (at head)

// add node (at tail)

// insert node

// remove node
    
// print list

    
