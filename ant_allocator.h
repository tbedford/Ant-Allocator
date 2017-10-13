#ifndef _ANT_ALLOCATOR_H_
#define _ANT_ALLOCATOR_H_

#define MIN_FRAG 64 // bytes
#define BLOCKHDR_SZ 16 // platform-specific

// Nabbed idea from Xinu
#define roundmb(s)  (size_t)((BLOCKHDR_SZ-1) + (size_t)(s)) & (~(BLOCKHDR_SZ-1)) 
#define truncatemb(s) (size_t)( (size_t)(s) & (~(BLOCKHDR_SZ-1)) )

typedef enum {false, true} bool;

typedef struct memobj_s {
    void * memptr;
    size_t request_sz;
} memobj_t;

// Memory block header for free block
typedef struct block_s {
    struct block_s *next;   
    size_t block_sz; // includes block header
} block_t;

typedef struct heap_s {
    size_t sysmem_sz; // size of memory grabbed from system
    void *sysmem; // pointer to grabbed system memory
} heap_t;

// global heap
heap_t heap; 
block_t freelist;

// Prototypes
void heap_create (size_t heap_sz);
void heap_destroy();
void dump_heap ();

void * ant_alloc (size_t request_sz);
bool ant_free (void *memptr, size_t request_sz);

#endif
