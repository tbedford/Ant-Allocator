#include <stdio.h>
#include "ant_allocator.h"

void test1 ()
{
    memobj_t memobj[8];
    
    heap_create(9000);

    size_t request_sz = 1000;
    for (int i=0; i<8; i++){
        memobj[i].request_sz = request_sz;
        memobj[i].memptr = ant_alloc(request_sz);
        if (memobj[i].memptr == NULL){
            printf("Allocation failed.\n");
        }
    }
    
    heap_dump();

    for (int i=0; i<8; i++){
        if (!ant_free(memobj[i].memptr, memobj[i].request_sz))
        {
            printf("Free failed.\n");
        }
    }

    heap_dump();
    heap_destroy();
}

void test2 ()
{
    size_t heap_sz = 9000;
    size_t request_sz = 1000;

    heap_create(heap_sz);

    void *p1 = ant_alloc(request_sz);
    void *p2 = ant_alloc(request_sz);
    void *p3 = ant_alloc(request_sz);
    void *p4 = ant_alloc(request_sz);
    
    heap_dump();

    ant_free(p2, request_sz);
    ant_free(p4, request_sz);
    
    heap_dump();

    ant_free(p1, request_sz);
    ant_free(p3, request_sz);

    heap_dump();

    heap_destroy();
}

int main (int argc, char **argv)
{
//    test1();
    test2();
    
    return 0;
}
