#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#define MIN_FRAG 128

typedef unsigned char byte;
typedef enum { false, true } bool;

typedef struct block_s {
    unsigned int id; // check for corruption
    void *user_mem; // useable memory (NULL if free block)
    size_t user_mem_sz; // size of usable memory 
    struct block_s *next;
    struct block_s *prev;
} block_t;


typedef struct heap_s {
    block_t *head; // points to start of whole heap (and first block in list)
    block_t *tail; // last node in the list
    size_t sys_mem_sz; // size of memory grabbed from system 
    void *sys_mem; // pointer to grabbed system memory
    void *wilderness; // will change
    size_t wilderness_sz; // shrinks
} heap_t;


void create_heap (heap_t *h, size_t s)
{
    h->sys_mem = malloc(s); // replace by kernel system call
    h->sys_mem_sz = s;
    
    h->head = NULL;
    h->tail = NULL;

    h->wilderness = h->sys_mem; // wilderness size will change
    h->wilderness_sz = h->sys_mem_sz;
}

void destroy_heap(heap_t *h)
{
    free(h->sys_mem); // replace by system kernel call
}

// Allocate space from wilderness
block_t * wild_alloc (heap_t *h, size_t user_mem_sz)
{
    
    size_t block_sz =  user_mem_sz + sizeof(block_t); // total block size required

    if (block_sz > h->wilderness_sz)
    {
        printf("ERROR: wilderness exhausted.\n");
        return NULL;
    }
    
    void *p = h->wilderness;
    h->wilderness = p + block_sz; // set new wilderness pointer
    h->wilderness_sz = h->wilderness_sz - block_sz;
    
    return (block_t *)p; // ptr to block
}

// creating a new block from wilderness
// user_mem_sz is user requested mem size from ant_alloc()
block_t *create_block (heap_t *h, size_t user_mem_sz)
{
    block_t *blk = wild_alloc (h, user_mem_sz);

    if (blk == NULL)
        return NULL;
    
    blk->id = 0x1D00D1; // for heap checking / testing  
    blk->user_mem_sz = user_mem_sz;
    blk->user_mem = (void *) blk + sizeof(block_t);
    blk->next = NULL;
    blk->prev = NULL;

    return blk;
}

// add block to end of list
void add_block (heap_t *h, block_t *blk)
{

    if (h->head == NULL) // list is empty
    {
        h->head = blk;
        h->tail = blk;
    }
    else // add after tail
    {
        blk->prev = h->tail;
        blk->next = NULL;
        h->tail->next = blk;
        h->tail = blk;
    }
    
}

bool check_mem_size (block_t *blk, size_t user_mem_sz)
{

    if (blk->user_mem_sz >= user_mem_sz)
    {
        if (blk->user_mem == NULL) // if free block
        {
            printf ("Free block found!\n"); // Debug only
            return true;
        }
    }

    return false;
}


// Find a free block in the block list
// Returns pointer to block or
// Returns NULL if no suitable free block found.
// If no block in block list we then have to
// make a new block from the wilderness chunk.
block_t * find_free_block (heap_t *h, size_t user_mem_sz)
{

    if (h->head == NULL) // empty list
        return NULL; 

    block_t *rover = h->head;
        
    do {
        // check block size
        if (check_mem_size (rover, user_mem_sz))
        {
            return rover; // return pointer to *BLOCK*
        }
        rover = rover->next;
    }
    while (rover != NULL); // until end of list

    return NULL; // No block found
}


bool check_prev_free (block_t *b)
{
    block_t *p = b->prev;

    if (p->user_mem == NULL)
        return true;
    return false;
}

bool check_next_free (block_t *b)
{
    block_t *p = b->next;

    if (p->user_mem == NULL)
        return true;
    return false;
}

void coalesce_block ()
{
    // TODO
}


// TODO ????
// splits a free block into an allocated block and a new free block if possible
// adds new free block into list for us
// if can't split block this does nothing
// user_mem_sz is how much of the block we need, frag is the bit we don't
// and which can be turned into a new free block if big enough.
void split_block (block_t *blk, size_t user_mem_sz)
{
    size_t block_sz = blk->user_mem_sz + sizeof(block_t);
    size_t frag_sz = block_sz - user_mem_sz - sizeof(block_t);; 
    
    if (frag_sz > MIN_FRAG) // we can split block
    {
        /*
          See docs for description.
          P3 = P1->Next
          P1->Next = P2
          P2->Prev = P1
          P2->Next = P3
          P3->Prev = P2
        */

        block_t *p1, *p2, *p3; // not really needed, but makes code more readable
        p1 = blk;
        p2 = blk + sizeof(block_t) + user_mem_sz;
        p3 = blk->next;

        p1->next = p2;
        p2->prev = p1;
        p2->next = p3;
        p3->prev = p2;

        // config rest of new block, p2
        p2->user_mem_sz = frag_sz - sizeof(block_t);
        p2->user_mem = NULL; // free block
        p2->id = 0x1D00D1;

        // Adjust p1 (was free block, now used)
        assert (user_mem_sz == (block_sz - sizeof(block_t) - frag_sz));
        p1->user_mem_sz = block_sz - sizeof(block_t) - frag_sz;
        p1->user_mem = p1 + sizeof(block_t);
    }
}


// return ptr to user memory
// or NULL on failure to find block
void * ant_alloc (heap_t *h, size_t user_mem_sz)
{
    block_t *blk = NULL;

    // try to reuse a block
    blk = find_free_block(h, user_mem_sz);
    if (blk != NULL) // we found a free block
    {
        // split block if possible

        split_block(blk, user_mem_sz);
          
        return blk->user_mem;
    }
    else // allocate from wilderness
    {
        blk = create_block(h, user_mem_sz);

        if (blk == NULL)
            return NULL; // alloc failed
        add_block(h, blk);
        return blk->user_mem;
    }
}


void ant_free (void *p)
{
    // TODO coalesce blocks
    
    block_t *blk = p - sizeof(block_t);
    blk->user_mem = NULL;
}

void check_heap (heap_t *h)
{
    printf("--- Check heap: ---\n");
    
    if (h->head == NULL)
    {
        printf("Empty list.\n");

    }
    else
    {
        block_t *rover = h->head;
        
        do {
            if (rover->id != 0x1D00D1)
            {
                printf("Block list is corrupted!\n");
            }
            rover = rover->next;
        }
        while (rover != NULL);
    }
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
        block_t *rover = h->head;
        size_t i = 0;
        
        do {
            printf ("ID: %X\n", (unsigned int)rover->id);
            printf ("User Mem_sz: %zu\n", rover->user_mem_sz);
            printf ("User Mem: %p\n", rover->user_mem);
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

    heap_t heap;
    size_t heap_sz = 16000;

    printf("Block_t size: %lu\n", sizeof(block_t));
    
    create_heap(&heap, heap_sz); 
       
    void *p1 = ant_alloc(&heap, 2000);
    void *p2 = ant_alloc(&heap, 1000);
    void *p3 = ant_alloc(&heap, 3000);

    check_heap(&heap);
    ant_free(p2);
    
    void *p4 = ant_alloc(&heap, 500);
 
      
    dump_heap(&heap);
    
    check_heap(&heap);
    destroy_heap (&heap);


    return 0;
}

