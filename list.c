#include <stdio.h>
#include <stdlib.h>

// !!! No error checking !!!

typedef unsigned int data_t;

typedef enum { false, true } bool;

typedef struct node_s {

    struct node_s *next;
    struct node_s *prev;
    data_t data;
    
} node_t;

typedef struct list_s {
    node_t *head;
    node_t *tail;
    size_t items; // number of nodes in list
} list_t;

// create list

void create_list (list_t *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->items = 0;
}

bool empty_list (list_t *list)
{
    if (list->head == NULL)
        return true;
    return false;
}
    
// create node

node_t *create_node (data_t data)
{
    node_t *n = (node_t *) malloc (sizeof(node_t));
    n->next = NULL;
    n->prev = NULL;
    n->data = data;

    return n;
}

// add node n to end of list l 
void add_node (list_t *l, node_t *n)
{
    if (l->items == 0) // empty list
    {
        l->head = n;
        l->tail = n;
        l->items = 1;
    }
    else // add after tail
    {
        n->prev = l->tail;
        n->next = NULL;
        l->tail->next = n;
        l->tail = n;
    }
    
}

// insert node n after node p

void insert_node (list_t *list, node_t *n, node_t *p)
{
        node_t *temp = p->next;
        temp->prev = p;
        p->next = n;
        n->next = temp;
        n->prev = p;
}

// remove node (from point p)

void remove_node (node_t *n)
{
    // TODO
}

// print list

void print_list (list_t *l)
{

    if (l->items == 0)
    {
        printf("Empty list.\n");

    }
    else
    {
        node_t *rover = l->head;

        do {
            printf ("Data: %d\n", (unsigned int)rover->data);
            rover = rover->next;
        }
        while (rover != NULL);
    }
}

// destroy node

void destroy_node (node_t *n)
{
    free (n);
}

// destroy list

void destroy_list (list_t *l)
{
    if (l->items == 0) return; // nothing to destroy

    node_t *rover = l->head;
    node_t *node;
    
    do {
        node = rover;
        free (node);
        rover = rover->next;
    }
    while (rover != NULL);
    printf("List destroyed.\n");
}

// main

int main (int c, char **argv)
{

    list_t list;
    node_t *node;
    
    create_list (&list);

    print_list (&list);
    
    node = create_node (12);
    add_node(&list, node);

    print_list (&list);
    
    node = create_node (34);
    add_node(&list, node);

    node = create_node (56);
    add_node(&list, node);

    printf("===\n");
    print_list(&list);

    destroy_list(&list);
    
    return 0;
}

    
