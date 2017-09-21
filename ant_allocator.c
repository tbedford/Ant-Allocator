#include<stdio.h>
#include<stdlib.h>

// a block is node + mem
// wilderness managed separately

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
    
    n->id = 0x1D4A11; // for heap checking / testing  
    n->mem_sz = mem_sz;
    n->mem = (void *) n + sizeof(node_t);
    n->free = true;
    n->next = NULL;
    n->prev = NULL;

    return n;
}

node_t * find_free_node (size_t size)
{
    // TODO

    return NULL;
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
    
    
    destroy_heap (&heap);
    
    return 0;
}
