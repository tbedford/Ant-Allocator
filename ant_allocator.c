#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "ant_allocator.h"

void heap_create (size_t heap_sz)
{
    // system available mem map might not be multiple of BLOCKHDR_SZ
    printf ("Heap size requested: %lu\n", heap_sz);
    heap_sz = truncatemb(heap_sz);
    printf ("Heap size truncated: %lu\n", heap_sz);
    
    heap.sysmem = malloc(heap_sz); // replace by kernel system call
    heap.sysmem_sz = heap_sz;

    // One block on list (some platforms may have multiple initial blocks)
    freelist.next = (block_t *) heap.sysmem;
    freelist.next->block_sz = heap.sysmem_sz;
    freelist.next->next = NULL;
}

void heap_destroy()
{
    free(heap.sysmem); // replace by system kernel call
}


void * ant_alloc (size_t request_sz)
{
    block_t *prev, *rover, *frag;

    if (request_sz == 0)
    {
        printf("You requested 0 bytes.\n");
        return NULL;
    }

    request_sz = roundmb(request_sz);
    printf ("Rounded request size to: %lu\n", request_sz);
    
    prev = &freelist;
    rover = freelist.next;

    while (rover != NULL)
    {
        if (rover->block_sz == request_sz)
        {
            printf("Exact size match!\n");
            prev->next = rover->next;
            freelist.block_sz = freelist.block_sz - request_sz;

            return (void *)(rover);
        }
        else if (rover->block_sz > request_sz)
        {
            printf("Found block larger than requested size.\n");
            // split block
            frag = (block_t *)((size_t)rover + request_sz);
            prev->next = frag;
            frag->next = rover->next;
            frag->block_sz = rover->block_sz - request_sz;
            freelist.block_sz = freelist.block_sz - request_sz;

            return (void *)(rover);
        }
        prev = rover;
        rover = rover->next;
    }
    printf("Couldn't find suitable block.\n");
    return NULL;    
}

// NOTE: app must keep track of orogonal request size in order to free!!
// Could change this to use memobj for convenience
bool ant_free (void *memptr, size_t request_sz)
{

    block_t *block, *next, *prev;
    size_t top;

    // make sure block is in heap
    if ((request_sz == 0)
        || ((size_t)memptr < (size_t)heap.sysmem)
        || ((size_t)memptr > (size_t)heap.sysmem + heap.sysmem_sz))
    {
        return false;
    }

    block = (block_t *)memptr;
    request_sz = (size_t)roundmb(request_sz);

    prev = &freelist;
    next = freelist.next;
    while ((next != NULL) && (next < block))
    {
        prev = next;
        next = next->next;
    }

    // find top of previous block
    if (prev == &freelist)
    {
        top = (size_t) NULL;
    }
    else
    {
        top = (size_t)prev + prev->block_sz;
    }

    // make sure block is not overlapping on prev or next blocks
    if ((top > (size_t)block)
        || ((next != NULL) && ((size_t)block + request_sz) > (size_t)next))
    {
        return false;
    }

    freelist.block_sz = freelist.block_sz + request_sz;

    // coalesce with previous block if adjacent
    if (top == (size_t)block)
    {
        prev->block_sz = prev->block_sz + request_sz;
        block = prev;
    }
    else
    {
        block->next = next;
        block->block_sz = request_sz;
        prev->next = block;
    }

    // coalesce with next block if adjacent
    if (((size_t)block + block->block_sz) == (size_t)next)
    {
        block->block_sz = block->block_sz + next->block_sz;
        block->next = next->next;
    }

    return true;    
}

void heap_dump ()
{
    size_t total_sz = 0;
    block_t *rover = freelist.next;
    size_t i = 0;

    printf("--- Listing free blocks: ---\n");

    if (freelist.next == NULL)
    {
        printf("No free blocks in list.\n");
    }
    else {
        do {
            // block size and block ptr
            printf ("%zu \t %p\n", rover->block_sz, rover);
            total_sz = total_sz + rover->block_sz;
            rover = rover->next;
            i++;
        }
        while (rover != NULL);

        printf("--- %zu free blocks in list. ---\n", i);
        printf("--- Total size of free blocks in list: %zu ---\n", total_sz);
    }
}


