#include<stdio.h>
#include<stdlib.h>

// a block is node + mem
// wilderness managed separately (this is nice because if this get low we can go
// cap in hand to the system to extend)
// grep TODO

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
} heap_t;


void * create_heap (heap_t *h, size_t s)
{
    h->sysmem = malloc(s);
    h->sysmem_sz = s;
    
    h->head = NULL;
    h->tail = NULL;

    return h->sysmem; // wilderness size will change
}

void destroy_heap(heap_t *h)
{
    free(h->sysmem);
}

void * wild_alloc (heap_t *h, size_t s)
{
    void *w = h->wilderness;
    h->wilderness = w + s; // set new wilderness pointer

    return w; // ptr to block
}

// creating a new list node
// mem_sz is user requested mem size from ant_alloc()
node_t *create_node (heap_t *h, size_t mem_sz)
{
    node_t *n = (node_t *) wild_alloc (h, mem_sz + sizeof(node_t));
    
    n->id = 0x1D00D1; // for heap checking / testing  
    n->mem_sz = mem_sz;
    n->mem = (void *) n + sizeof(node_t);
    n->free = true;
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
        return true;
    return false;
    
}


// find a node that had suitable memory available
// s is mem_sz - that is user requested size
// in the simple case we find first node with mem_sz > s
// this is first fit. We then have the problem of what to do about
// the wasted memory if mem_sz is a lot smaller than s
// (turn it into a new free block)
// Returns NULL if no suitable block found. We then have to
// make a new node from the wilderness chunk.
node_t * find_free_node (heap_t *h, size_t s)
{

    if (h->head == NULL) // empty list
        return NULL;

    node_t *rover = h->head;
        
    do {
        // check block size
        if (check_mem_size (rover, s))
            return rover;
        rover = rover->next;
    }
    while (rover != NULL); // end of list

    return NULL; // No block found
}

void dump_heap (heap_t *h)
{
    if (h->head == NULL)
    {
        printf("Empty list.\n");

    }
    else
    {
        node_t *rover = h->head;
        size_t i = 0;
        
        do {
            printf ("ID: %d\n", (unsigned int)rover->id);
            printf ("Mem_sz: %zu\n", rover->mem_sz);
            rover = rover->next;
            i++;
        }
        while (rover != NULL);

        printf("%zu items in list.\n", i);
    }

}


// return ptr to usable memory 
void * ant_alloc (size_t mem_sz)
{
    // TODO

    return 0;
}

void ant_free (void *p)
{
    // NOP
}

int main (int argc, char **argv)
{
    printf("Ant allocator!\n");
    printf("Size of size_t %lu!\n", sizeof(size_t));

    heap_t heap;
    size_t heap_sz = 8000;

    create_heap(&heap, heap_sz); 


    // do stuff

    void *mem = ant_alloc(1000);

    dump_heap(&heap);
    
    destroy_heap (&heap);
    
    return 0;
}
