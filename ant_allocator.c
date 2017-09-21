#include<stdio.h>
#include<stdlib.h>

typedef unsigned char byte;
typedef enum { false, true } bool;

typedef struct block_s {
    unsigned int id;
    void *mem; // useable memory
    size_t mem_sz; // of mem (block_sz = sizeof(block_t)+mem_sz)
    bool free;
    struct block_s *next;
    struct block_s *prev;
} block_t;

typedef struct heap_s {
    block_t *head; // points to start of whole heap (and first block in list)
    block_t *tail;
    size_t heap_sz;
    void *heap;
} heap_t;

void block_init (void *block, size_t block_sz);

void heap_init (heap_t *hp, size_t hs)
{
    hp->heap = malloc(hs);
    hp->heap_sz = hs;

    // At the outset, the heap is one big free block
    block_init(hp->heap, hs);

    hp->head = hp->heap;
    hp->tail = hp->head;
}

void heap_free(void *hp)
{
    free(hp);
}

// Given a pointer and a size we can turn this free space into a block
void block_init (void *block, size_t block_sz)
{
    block_t *bp = (block_t *) block;
    bp->id = 0x1234; // for heap checking / testing  
    bp->mem_sz = block_sz - sizeof(block_t);
    bp->free = true;
    bp->next = NULL;
    bp->prev = NULL;

}

block_t * find_free_block (size_t size)
{


    
}

// return ptr to usable memory 
void * ant_alloc (size_t size)
{
    

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
    heap_init(&heap, heap_sz); 


    // do stuff
    
    
    heap_free (heap.heap);
    
    return 0;
}
