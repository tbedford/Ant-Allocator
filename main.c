#include <stdio.h>

#include "ant_allocator.h"

int main (int argc, char **argv)
{
    memobj_t mo[8];
    
    heap_create (8000);

    // allocate
    for (int i = 0; i < 8; i++){
        if (ant_alloc(&mo[i], 1000))    {
            printf("%d: Allocated OK\n", i);
        }
    }

    dump_heap();

    // free
    for (int i = 0; i < 8; i++){
        if (ant_free(&mo[i]))    {
            printf("%d: Freed OK\n", i);
        }
    }

    dump_heap();
    
    heap_destroy ();
    
    return 0;
}
