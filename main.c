#include <stdio.h>

#include "ant_allocator.h"

// PURPOSE: simple alloc and free in sequence
void test1()
{
    allocator_t alloc;
    size_t heap_sz = 16000;

    allocator_create(&alloc, heap_sz);

    void *p1 = ant_alloc(&alloc, 2000);
    void *p2 = ant_alloc(&alloc, 1000);
    void *p3 = ant_alloc(&alloc, 3000);
    
    ant_free (&alloc, p1);
    ant_free (&alloc, p2);
    ant_free (&alloc, p3);
    
    dump_heap(&alloc);
    check_heap(&alloc);
    allocator_destroy (&alloc);

}

// PURPOSE: test case where we free middle block first
void test2()
{
    allocator_t alloc;
    size_t heap_sz = 16000;

    allocator_create(&alloc, heap_sz);

    void *p1 = ant_alloc(&alloc, 2000);
    void *p2 = ant_alloc(&alloc, 1000);
    void *p3 = ant_alloc(&alloc, 3000);
    
    ant_free (&alloc, p2);
    ant_free (&alloc, p3);
    ant_free (&alloc, p1);
    
    dump_heap(&alloc);
    check_heap(&alloc);
    allocator_destroy (&alloc);

}


// PURPOSE: test both block splitting and block coalescing
void test3 ()
{
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
}


// Main

int main (int argc, char **argv)
{
    // This could be useful for tracing
//    printf ("FILE:%s LINE:%d\n", __FILE__, __LINE__);

    test1();
    test2();
    test3();
    
    return 0;
}
