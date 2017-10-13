#ifndef _ANT_ALLOCATOR_H_
#define _ANT_ALLOCATOR_H_

#define MIN_FRAG 64 // bytes

typedef enum { false, true } bool;

// Memory block header
typedef struct blockhdr_s {
    struct blockhdr_s *next;
    size_t usermem_sz; // size of usable memory
} blockhdr_t;


// list of FREE blocks
typedef struct blocklist_s {
    blockhdr_t *head; // points to start of first block in list
    size_t sysmem_sz; // size of memory grabbed from system
    void *sysmem; // pointer to grabbed system memory
    size_t total_allocated_memory; // for debugging
} blocklist_t;

// Memory object
typedef struct memobj_s {
    void *ptr;
    size_t size;
} memobj_t;

// File scope
static size_t BLOCKHDR_SZ;// Not ideal - but otherwise multiple calls to sizeof()

// Prototypes
void list_create (blocklist_t *list, size_t heap_sz);
void list_destroy(blocklist_t *list);

bool ant_alloc (memobj_t *memobj, size_t usermem_sz);
bool ant_free (memobj_t *memobj);

void dump_heap (blocklist_t *list);


#endif
