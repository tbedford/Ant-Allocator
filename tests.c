//
// Ant Allocator tests
//

// test wilderness exhaustion
void test1 ()
{

    heap_t heap;
    size_t heap_sz = 8000;

    create_heap(&heap, heap_sz); 
    dump_heap(&heap);
        
    void *p1 = ant_malloc(&heap, 1000);
    void *p2 = ant_malloc(&heap, 2000);
    void *p3 = ant_malloc(&heap, 4000);

    dump_heap(&heap);
    
    void *p4 = ant_malloc(&heap, 4000);
    if (p4 == NULL)
    {
        printf ("Allocation failed.\n");
    }

    dump_heap(&heap);

    destroy_heap (&heap);
}


// check ant_free()
void test2 ()
{

    heap_t heap;
    size_t heap_sz = 8000;

    create_heap(&heap, heap_sz); 
         
    void *p1 = ant_alloc(&heap, 1000);
    void *p2 = ant_alloc(&heap, 2000);
    void *p3 = ant_alloc(&heap, 3000);
    void *p4 = ant_alloc(&heap, 1000);

    ant_free(p2);
    
    dump_heap(&heap);
    
    destroy_heap (&heap);
}

// check free block resuse
void test3 ()
{

    heap_t heap;
    size_t heap_sz = 8000;

    create_heap(&heap, heap_sz); 
         
    void *p1 = ant_alloc(&heap, 2000);
    void *p2 = ant_alloc(&heap, 1000);
    void *p3 = ant_alloc(&heap, 3000);

    dump_heap(&heap);
    
    ant_free(p2);
    
    void *p4 = ant_alloc(&heap, 1000);

    dump_heap(&heap);
    
    destroy_heap (&heap);
}
