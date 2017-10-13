#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "ant_allocator.h"

void heap_create (size_t heap_sz)
{

    heap.sysmem = malloc(heap_sz); // replace by kernel system call
    heap.sysmem_sz = heap_sz;

    // One block on list (some platforms may have multiple initial blocks)
    heap.head = (block_t *) heap.sysmem;
    heap.head->next = NULL;
    heap.head->block_sz = heap.sysmem_sz;
}

void heap_destroy()
{
    free(heap.sysmem); // replace by system kernel call
}


bool ant_alloc (memobj_t *memobj, size_t requested_sz)
{
    if ((memobj == NULL)||(requested_sz == 0)||(heap.head == NULL))
    {
        return false;
    }
    
    memobj->requested_sz = requested_sz; 

    // adjust requested size here??
    
    block_t *rover = heap.head;
    block_t *prev = NULL;

    size_t frag_sz = 0;
    
    do {
        if (rover->block_sz == requested_sz)
        {
            memobj->mem = rover;
        }
        else if (rover->block_sz > requested_sz) 
        {
            frag_sz = rover->block_sz - requested_sz;
            if (frag_sz > MIN_FRAG)
            {
                rover->block_sz = rover->block_sz - requested_sz;
                memobj->mem = rover + (rover->block_sz);
                return true;
            }
            else
            {
                memobj->mem = rover;
            }
        }
        else
        {
            return false; // can't find free block of suitable size
        }

        // fix up list pointers
        if (prev != NULL){
            prev->next =  rover->next;    
        }
        else
        { // if start of list
            if (rover->next == NULL)
            {
                heap.head = NULL;
            }
            else
            {
                heap.head = rover->next;
            }
        }

        return true;

        prev = rover;
        rover = rover->next;
    }
    while (rover != NULL); // until end of list
}

bool ant_free (memobj_t *memobj)
{
    // TODO
    return true;
}

void dump_heap ()
{
    printf("--- Listing free blocks: ---\n");

    block_t *rover = heap.head;
    size_t i = 0;

    if (heap.head == NULL)
    {
        printf("No free blocks in list.\n");
    }
    else {
        do {
            // block size and block ptr
            printf ("%zu \t %p\n", rover->block_sz, rover);
            rover = rover->next;
            i++;
        }
        while (rover != NULL);

        printf("--- %zu free blocks in list. ---\n", i);
    }
}


