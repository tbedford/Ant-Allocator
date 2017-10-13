#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "ant_allocator.h"

void list_create (blocklist_t *list, size_t heap_sz)
{

    BLOCKHDR_SZ = sizeof(blockhdr_t); // set global value

    list->sysmem = malloc(heap_sz); // replace by kernel system call
    list->sysmem_sz = heap_sz;

    // One free block on list
    list->head = (blockhdr_t *)sysmem;
    list->head->next = NULL;
    list->head->usermem_sz = sysmem_sz - BLOCKHDR_SZ;
}

void list_destroy(blocklist_t *list)
{
    free(alloc->sysmem); // replace by system kernel call
}


// Find a free block of suitable size in the block list
// Returns pointer to BLOCK or
// Returns NULL if no suitable free block found.
blockhdr_t * find_block (blocklist_t *list, size_t usermem_sz)
{
    if (list->head == NULL) // empty list
        return NULL;

    blockhdr_t *rover = list->head;
    blockhdr_t *prev = NULL;
    blockhdr_t *free_block = NULL;

    do {
        if (rover->usermem_sz == usermem_sz)
        {
            return rover; 
        }
        else if (rover->usermem_sz > usermem_sz)             // split block
        {
            size_t block_sz = BLOCKHDR_SZ + rover->usermem_sz;
            free_block = rover + usermem_sz;
            free_block->usermem_sz = block_sz - usermem_sz - BLOCKHDR_SZ;
            
            if (prev == NULL) // first block in list
            {
                free_block->next = rover->next;
                list->head = free_block; 
            }
            else // not first in list
            {
                // TODO
            }
        }
        else
        {
            return NULL;
        }
        rover = rover->next;
        prev = rover;
    }
    while (rover != NULL); // until end of list
}


void split_block (blocklist_t *list, blockhdr_t *blk, size_t usermem_sz)
{
    size_t block_sz = (blk->usermem_sz) + BLOCKHDR_SZ;
    size_t frag_sz = block_sz - usermem_sz - BLOCKHDR_SZ;

    if (frag_sz > MIN_FRAG) // we can split block
    {
        // redo code
    }
}

void coalesce_blocks ()
{
    // TODO
}

bool ant_alloc (memobj_t *memobj, size_t usermem_sz)
{

}

bool ant_free (memobj_t *memobj)
{
    // todo
}


void dump_heap (blocklist_t *list)
{
    printf("--- Dump free blocks list: ---\n");

    if (list->head == NULL)
    {
        printf("Empty list.\n");
    }
    else
    {
        blockhdr_t *rover = list->head;
        size_t i = 0;

        do {
            // User_mem_sz | User_mem_ptr
            printf ("%zu \t %p \t %d\n", rover->user_mem_sz, rover + BLOCKHDR_SZ);
            rover = rover->next;
            i++;
        }
        while (rover != NULL);

        printf("--- %zu items in list. ---\n", i);
    }
}


