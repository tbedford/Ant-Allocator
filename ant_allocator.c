#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#define MIN_FRAG 128

typedef enum { false, true } bool;

typedef struct block_hdr_s {
    unsigned int id; // check for corruption
    size_t user_mem_sz; // size of usable memory
    bool free; // true if free block
    struct block_hdr_s *next;
    struct block_hdr_s *prev;
} block_hdr_t;


typedef struct allocator_s {
    block_hdr_t *head; // points to start of first block in list
    block_hdr_t *tail; // last block in the list (before wilderness)
    size_t sys_mem_sz; // size of memory grabbed from system
    void *sys_mem; // pointer to grabbed system memory
    void *wilderness; // will change
    size_t wilderness_sz; // shrinks
} allocator_t;

// GLOBAL
size_t BLOCK_HDR_SZ;// Not ideal - but otherwise multiple calls to sizeof()

void allocator_create (allocator_t *alloc, size_t s)
{
    alloc->sys_mem = malloc(s); // replace by kernel system call
    alloc->sys_mem_sz = s;

    alloc->head = NULL;
    alloc->tail = NULL;

    alloc->wilderness = alloc->sys_mem; // wilderness size will change
    alloc->wilderness_sz = alloc->sys_mem_sz;
}

void allocator_destroy(allocator_t *alloc)
{
    free(alloc->sys_mem); // replace by system kernel call
}

// Allocate space from wilderness
block_hdr_t * wild_alloc (allocator_t *alloc, size_t user_mem_sz)
{
    size_t block_sz =  user_mem_sz + BLOCK_HDR_SZ; // total block size required

    if (block_sz > alloc->wilderness_sz)
    {
        printf("ERROR: wilderness exhausted.\n");
        return NULL;
    }

    void *p = alloc->wilderness;
    alloc->wilderness = p + block_sz; // set new wilderness pointer
    alloc->wilderness_sz = alloc->wilderness_sz - block_sz;

    return (block_hdr_t *)p; // ptr to block
}

// creating a new block from wilderness
// user_mem_sz is user requested mem size from ant_alloc()
// Returns NULL if wild_alloc failed to provide
block_hdr_t *create_block (allocator_t *alloc, size_t user_mem_sz)
{
    block_hdr_t *blk = wild_alloc (alloc, user_mem_sz);

    if (blk == NULL)
        return NULL;

    blk->id = 0x1D00D1; // for heap checking / testing
    blk->user_mem_sz = user_mem_sz;
    blk->free = false;
    blk->next = NULL;
    blk->prev = NULL;

    return blk;
}

// add block to end of list
void add_block (allocator_t *alloc, block_hdr_t *blk)
{

    if (alloc->head == NULL) // list is empty
    {
        alloc->head = blk;
        alloc->tail = blk;
    }
    else // add after tail
    {
        blk->prev = alloc->tail;
        blk->next = NULL;
        alloc->tail->next = blk;
        alloc->tail = blk;
    }

}

// Find a free block in the block list
// Returns pointer to block or
// Returns NULL if no suitable free block found.
// If no block in block list we then have to
// make a new block from the wilderness chunk.
block_hdr_t * find_free_block (allocator_t *alloc, size_t user_mem_sz)
{

    if (alloc->head == NULL) // empty list
        return NULL;

    block_hdr_t *rover = alloc->head;

    do {
        // free block of sufficient size?
        if (rover->free && (rover->user_mem_sz >= user_mem_sz))
        {
            return rover; // return pointer to *BLOCK*
        }
        rover = rover->next;
    }
    while (rover != NULL); // until end of list

    return NULL; // No block found
}



// splits a free block into an allocated block
// and a new free block if possible
// adds new free block into list for us
// if can't split block this does nothing
// user_mem_sz is how much of the block we need, frag is the bit we don't
// and which can be turned into a new free block if big enough.
void split_block (block_hdr_t *blk, size_t user_mem_sz)
{
    size_t block_sz = (blk->user_mem_sz) + BLOCK_HDR_SZ;
    size_t frag_sz = block_sz - user_mem_sz - BLOCK_HDR_SZ;

    if (frag_sz > MIN_FRAG) // we can split block
    {
        block_hdr_t *p1, *p2, *p3; // not really needed, but makes code more readable
        p1 = blk;
        p2 = blk + BLOCK_HDR_SZ + user_mem_sz;
        p3 = blk->next;

        p1->next = p2;
        p2->prev = p1;
        p2->next = p3;
        p3->prev = p2;

        // config rest of new block, p2
        p2->user_mem_sz = frag_sz - BLOCK_HDR_SZ;
        p2->free = true; // free block
        p2->id = 0x1D00D1;

        // Adjust p1 (was free block, now used)
        assert (user_mem_sz == (block_sz - BLOCK_HDR_SZ - frag_sz));
        p1->user_mem_sz = block_sz - BLOCK_HDR_SZ - frag_sz;
        p1->free = false;
    }
}


// return ptr to user memory
// or NULL on failure to find block
// TODO: add fprintf(stderr) tracing (silly because fprintf uses malloc()!)
void * ant_alloc (allocator_t *alloc, size_t user_mem_sz)
{
    block_hdr_t *blk = NULL;

    // try to reuse a block
    blk = find_free_block(alloc, user_mem_sz);
    if (blk != NULL) // we found a free block
    {
        // split block if possible

        split_block(blk, user_mem_sz);

        return blk + BLOCK_HDR_SZ;
    }
    else // allocate from wilderness
    {
        blk = create_block(alloc, user_mem_sz);

        if (blk == NULL)
            return NULL; // alloc failed
        add_block(alloc, blk);
        return blk + BLOCK_HDR_SZ; // return user mem
    }
}

/*
bool check_prev_free (block_hdr_t *b)
{
    block_hdr_t *p = b->prev;

    if (p->free)
        return true;
    return false;
}

bool check_next_free (block_hdr_t *b)
{
    block_hdr_t *p = b->next;

    if (p->free)
        return true;
    return false;
}
*/


// p2 is right-hand of two blocks, it will
// coalesce with the block on the left, p1
void coalesce_blocks (block_hdr_t * p2)
{
    block_hdr_t *p1 = p2->prev; // set left-hand block
    size_t bs1, bs2, new_block_sz;

    bs1 = (p1->user_mem_sz) + BLOCK_HDR_SZ;
    bs2 = (p2->user_mem_sz) + BLOCK_HDR_SZ;
    new_block_sz = bs1 + bs2;

    // coalesce left and right blocks
    p1->next = p2->next;
    p2->next->prev = p1;

    p1->user_mem_sz = new_block_sz - BLOCK_HDR_SZ;
}

void ant_free (void *p)
{
    // !!! BUG FIXED !!! This was a nasty bug -
    // You MUST cast p - the compiler will not warn you, at least
    // with default switches!
    block_hdr_t *blk = (block_hdr_t *)p - BLOCK_HDR_SZ;
    blk->free = true;

    // coalesce free blocks if possible
    // check left-hand block 
    if (blk->prev->free == true)
    {
        coalesce_blocks(blk);
    }
    // check right-hand block
    if (blk->next->free == true)
    {
        coalesce_blocks(blk->next); // always pass right hand block
    }
}

void check_heap (allocator_t *alloc)
{
    printf("--- Check heap: ---\n");

    if (alloc->head == NULL)
    {
        printf("Empty list.\n");

    }
    else
    {
        block_hdr_t *rover = alloc->head;

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


void dump_heap (allocator_t *alloc)
{
    printf("--- Dump heap: ---\n");

    if (alloc->head == NULL)
    {
        printf("Empty list.\n");

    }
    else
    {
        block_hdr_t *rover = alloc->head;
        size_t i = 0;

        do {
            // ID | User_mem_sz | User_mem_ptr | Free
            printf ("%X \t %zu \t %p \t %d\n", rover->id, rover->user_mem_sz, rover + BLOCK_HDR_SZ, rover->free);
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

    printf ("FILE:%s LINE:%d\n", __FILE__, __LINE__);
    
    BLOCK_HDR_SZ = sizeof(block_hdr_t);
    allocator_t allocator;
    size_t heap_sz = 16000;

    allocator_create(&allocator, heap_sz);
    void *p1 = ant_alloc(&allocator, 2000);
    void *p2 = ant_alloc(&allocator, 1000);
    void *p3 = ant_alloc(&allocator, 3000);
    
    ant_free (p2);

    void *p4 = ant_alloc(&allocator, 500);


    dump_heap(&allocator);
    check_heap(&allocator);
    allocator_destroy (&allocator);

    return 0;
}

