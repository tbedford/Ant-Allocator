#ifndef _ANT_ALLOCATOR_H_
#define _ANT_ALLOCATOR_H_


#define MIN_FRAG 64 // bytes, block overhead is 40 bytes

typedef enum { false, true } bool;

typedef struct block_hdr_s {
    unsigned int id; // check for corruption
    size_t user_mem_sz; // size of usable memory
    bool free; // true if free block
    struct block_hdr_s *next;
    struct block_hdr_s *prev;
} block_hdr_t;


typedef struct allocator_s {
    block_hdr_t *head; // points to start of first block in list
    block_hdr_t *tail; // last block in the list (before wilderness)
    size_t sys_mem_sz; // size of memory grabbed from system
    void *sys_mem; // pointer to grabbed system memory
    void *wilderness; // will change
    size_t wilderness_sz; // shrinks
} allocator_t;

// GLOBAL
size_t BLOCK_HDR_SZ;// Not ideal - but otherwise multiple calls to sizeof()

void allocator_create (allocator_t *alloc, size_t s);
void allocator_destroy(allocator_t *alloc);
void * ant_alloc (allocator_t *alloc, size_t user_mem_sz);
void ant_free (allocator_t *alloc, void *p);
void check_heap (allocator_t *alloc);
void dump_heap (allocator_t *alloc);


#endif
