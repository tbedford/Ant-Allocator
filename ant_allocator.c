#include<stdio.h>
#include<stdlib.h>

#include "tests.h"

typedef unsigned char byte;
typedef enum { false, true } bool;

typedef struct node_s {
    unsigned int id; // check for corruption
    void *mem; // useable memory
    size_t mem_sz; // size of usable memory 
    bool free; // used or free block
    struct node_s *next;
    struct node_s *prev;
} node_t;

typedef struct heap_s {
    node_t *head; // points to start of whole heap (and first block in list)
    node_t *tail; // last node in the list
    size_t sysmem_sz; // size of memory grabbed from system 
    void *sysmem; // pointer to grabbed system memory
    void *wilderness; // will change
    size_t wilderness_sz; // shrinks
} heap_t;


void create_heap (heap_t *h, size_t s)
{
    h->sysmem = malloc(s); // replace by kernel system call
    h->sysmem_sz = s;
    
    h->head = NULL;
    h->tail = NULL;

    h->wilderness = h->sysmem; // wilderness size will change
    h->wilderness_sz = h->sysmem_sz;
}

void destroy_heap(heap_t *h)
{
    free(h->sysmem);
}

// Allocate space from wilderness
node_t * wild_alloc (heap_t *h, size_t mem_sz)
{
    
    size_t block_sz =  mem_sz + sizeof(node_t); // total block size required

    if (block_sz > h->wilderness_sz)
    {
        // TODO: improve this, such as expanding wilderness
        printf("ERROR: wilderness exhausted.\n");
        return NULL;
    }
    
    void *p = h->wilderness;
    h->wilderness = p + block_sz; // set new wilderness pointer
    h->wilderness_sz = h->wilderness_sz - block_sz;
    
    return p; // ptr to block
}

// creating a new list node from wilderness
// mem_sz is user requested mem size from ant_alloc()
node_t *create_node (heap_t *h, size_t mem_sz)
{
    node_t *n = wild_alloc (h, mem_sz);

    if (n == NULL)
        return NULL;
    
    n->id = 0x1D00D1; // for heap checking / testing  
    n->mem_sz = mem_sz;
    n->mem = (void *) n + sizeof(node_t);
    n->free = false;
    n->next = NULL;
    n->prev = NULL;

    return n;
}

// add node to end of list
void add_node (heap_t *h, node_t *n)
{

    if (h->head == NULL) // list is empty
    {
        h->head = n;
        h->tail = n;
    }
    else // add after tail
    {
        n->prev = h->tail;
        n->next = NULL;
        h->tail->next = n;
        h->tail = n;
    }
    
}

bool check_mem_size (node_t *n, size_t s)
{

    if (n->mem_sz >= s)
    {
        if (n->free)
        {
            printf ("Free block found!\n"); // Debug
            return true;
        }
    }

    return false;
}


// Returns NULL if no suitable free block found. We then have to
// make a new node from the wilderness chunk.
void * find_free_block (heap_t *h, size_t mem_sz)
{

    if (h->head == NULL) // empty list
        return NULL; 

    node_t *rover = h->head;
        
    do {
        // check block size
        if (check_mem_size (rover, mem_sz))
        {
            return rover->mem;
        }
        rover = rover->next;
    }
    while (rover != NULL); // end of list

    return NULL; // No block found
}

// return ptr to usable memory
// or NULL on failure to find block
void * ant_malloc (heap_t *h, size_t s)
{
    void *p = NULL;

    // try to reuse a block
    p = find_free_block(h, s);
    if (p != NULL)
    {
        // split block
        
        return p;
    }
    else // allocate from wilderness
    {
        node_t *n = create_node(h, s);

        if (n == NULL)
            return NULL;
        add_node(h, n);
        return n->mem;
    }
}

bool check_prev_free (node_t *n)
{
    node_t *p = n->prev;

    if (p->free)
        return true;
    return false;
}

bool check_next_free (node_t *n)
{
    node_t *p = n->next;

    if (p->free)
        return true;
    return false;
}


void coalesce_block ()
{
    // TODO
}

bool is_block_splittable (node_t *n)
{
    // TODO
    
    return false;
}


node_t * split_block (node_t *n)
{
    // TODO

    return NULL;
}


void ant_free (void *p)
{
    // TODO coalesce blocks
    
    node_t *n = p - sizeof(node_t);
    n->free = true;
}


void dump_heap (heap_t *h)
{
    printf("--- Dump heap: ---\n");
    
    if (h->head == NULL)
    {
        printf("Empty list.\n");

    }
    else
    {
        node_t *rover = h->head;
        size_t i = 0;
        
        do {
            printf ("ID: %X\n", (unsigned int)rover->id);
            printf ("Mem_sz: %zu\n", rover->mem_sz);
            printf ("Free: %d\n", rover->free);
            printf ("Mem: %p\n", rover->mem);
            rover = rover->next;
            i++;
        }
        while (rover != NULL);

        printf("--- %zu items in list. ---\n", i);
    }

}


// Main

int main (int argc, char **argv)
{


    return 0;
}

