#ifndef _ANT_ALLOCATOR_H_
#define _ANT_ALLOCATOR_H_

#define MIN_FRAG 64 // bytes
#define BLOCKHDR_SZ 16 // platform-specific

typedef enum { false, true } bool;

// Memory block header for free block
typedef struct block_s {
    struct block_s *next;   
    size_t block_sz; // includes block header
} block_t;


// list of FREE blocks
typedef struct heap_s {
    block_t *head; // points to start of first block in list
    size_t sysmem_sz; // size of memory grabbed from system
    void *sysmem; // pointer to grabbed system memory
    size_t total_allocated_memory; // for debugging
} heap_t;

// Memory object
typedef struct memobj_s {
    void *mem;
    size_t requested_sz;
} memobj_t;

// global heap
heap_t heap; 

// Prototypes
void heap_create (size_t heap_sz);
void heap_destroy();
void dump_heap ();

bool ant_alloc (memobj_t *memobj, size_t usermem_sz);
bool ant_free (memobj_t *memobj);

#endif
