#include <stdio.h>

#include "ant_allocator.h"

// Main

int main (int argc, char **argv)
{
    printf ("FILE:%s LINE:%d\n", __FILE__, __LINE__);
    
    BLOCK_HDR_SZ = sizeof(block_hdr_t);
    allocator_t alloc;
    size_t heap_sz = 16000;

    allocator_create(&alloc, heap_sz);

    void *p1 = ant_alloc(&alloc, 2000);
    void *p2 = ant_alloc(&alloc, 1000);
    void *p3 = ant_alloc(&alloc, 3000);
    
    ant_free (&alloc, p2);

    void *p4 = ant_alloc(&alloc, 500);

    ant_free(&alloc, p3);

    void *p5 = ant_alloc(&alloc, 1500);

    ant_free(&alloc, p1);

    void *p6 = ant_alloc(&alloc, 1000);

    dump_heap(&alloc);
    
    void *p7 = ant_alloc(&alloc, 4000);
    ant_free (&alloc, p7);
    
    dump_heap(&alloc);
    check_heap(&alloc);
    allocator_destroy (&alloc);

    return 0;
}
