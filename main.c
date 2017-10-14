#include <stdio.h>
#include "ant_allocator.h"

int main (int argc, char **argv)
{
    memobj_t memobj[8];
    
    heap_create(9000);

    size_t request_sz = 1000;
    for (int i=0; i<8; i++){
        memobj[i].memptr = ant_alloc(request_sz);
        if (memobj[i].memptr == NULL){
            printf("Allocation failed.\n");
        }
    }
    
    heap_dump();
    heap_destroy();

    return 0;
}
