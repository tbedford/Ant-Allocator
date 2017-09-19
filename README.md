# Ant-Allocator

A very, very simple memory allocator for educational purposes.

The following diagram shows the basic design / data structures for a
simple heap allocator:

![Heap Memory Allocator](./images/Memory_allocator_1.png)

After struggling with pen and paper for a while, I realized that it
helps to visualize the blocks (used and free) as separate from the
nodes that manage the blocks (metadata).

The nodes contain a pointer to the block, the size of the block,
whether the block is used or not, and a couple of pointers to allow
the nodes to be hooked into a doubly-linked list.

A doubly-linked list seems a bit complex, but there are points where
you will need to check the previous and next blocks to see if they are
free and therefore can be coalesced with the current block. For
example, if you have free, used, free blocks and then you free the
used block you have free, free, free. These blocks can be coalesced
into one free block.

Note that, in the diagram, a `free()` has meant that there are two
contiguous free blocks (adjacent red blocks) that could be coalesced
into one free block, thus helping to avoid memory fragmentation.

Memory fragmentation occurs where you have a number of free blocks,
but they are not of sufficient size to service a `malloc()`. You
basically end up with memory that looks like Swiss cheese - lots of
little holes. Block coalescing is one way around this. There are other
techniques such as binning where you try to keep small allocations in
one area and larger allocations in another area. This way you will end
up with small holes in the small allocation area, that are likely to
be sufficient to service `malloc()` requests. You don't want the large
allocation area ful of small holes. For example you could end up with
say 32KB total free, but only be able to service a `malloc()` of
4KB. Not good.

In a real implementation the nodes (metadata) will be part of the
blocks, and not separate.

Basic node data structure would look something like (in dodgy
pseudo-code):

```C
// node for managing a block
typedef node_s struct {
    block_ptr *b; // points to a memory block
    unsigned int size; // size of block, look into size_t
    boolean_t used; // used: true or false
    node_s *next;
    node_s *prev;	   
 } node_t;
```

This is not the most efficient data structure though, but serves to
get a very basic design up and running.

## malloc()

Simple case: You would walk along the linked-list checking for the
first free block of suitable size. Unless the free block was of
exactly the right size, it would then be split into an allocated block
and a new free block in the list (with its new metadata). This
requires you insert a new node (a free block) into the
linked-list. This is basically the "first fit" approach.

## free()

Simple case: You are given the address of the block to free. In a real
implementation the meta-data (node) associated with an allocated block
would be part of the block itself. So given the address of the block
to free we can quickly look up metadata for the block, set used to
free, and coalesce any adjacent blocks, updating metadata as required.

## Blocks containing metadata

In practice the nodes for managing metadata would be part of the
allocated block. This is shown in a modified version of the previous
diagram:

![Heap Memory Allocator with nodes in block](./images/Memory_allocator_2.png)

Note that it is necessary to allocate larger than the requested block
size - that is, you need to allocate block size plus node size. You no
longer need to store the pointer to the block in a node (although I do
to make things easier). As you know the location of the node in memory
(either passed to free(), or from walking the list during malloc()),
you can simply add a constant to it (the node size) to find the start
point of the actual allocation.

If you were doing a `free()` you would take the passed pointer and
subtract the node size to get the start of the metadata.

## Really simple code

```C
#include<stdio.h>
#include<stdlib.h>

typedef unsigned char byte;

typedef enum { false, true } bool;

typedef struct node_s {
    void *block;
    size_t size; // of actual usable block
    bool free;
    struct node_s *next;
    struct node_s *prev;
} node_t;


struct list_s {
    node_t *head;
    node_t *tail;
};


int main (int argc, char **argv)
{
    printf("Ant allocator!\n");
    printf("Size of size_t %lu!\n", sizeof(size_t));

    struct list_s list;
    list.head = NULL;
    list.tail = NULL;

    size_t heap_sz = 8000;
    
    void *heap = malloc (heap_sz); // small heap to manage

    // create first node/block which is the whole heap
    node_t *p = heap;
    p->block = p + sizeof(node_t);
    p->size = heap_sz - sizeof(node_t);
    p->free = true;
    p->next = NULL;
    p->prev = NULL;

    list.head = p;
    list.tail = NULL;

    // at this point we have one node in the list
   
    p = list.head;
    byte *b = (byte *)p->block;
    printf("Block size: %lu\n", p->size);

    for (int i; i < p->size; i++)
    {
        *b = 0xAA;
        b++;
    }

    byte data = *((byte *)p->block);
    printf("Data: %X\n", data);
    
    free (heap);
    
    return 0;
}

```

## Alignment

TODO

## Sentinals

TODO

## Testing

TODO

## References

* [Jemalloc](https://linux.die.net/man/3/jemalloc)
* [size_t](https://stackoverflow.com/questions/2550774/what-is-size-t-in-c#2550799)
* [Fuschia/Magenta OS](https://github.com/fuchsia-mirror/magenta/blob/master/kernel/lib/heap/cmpctmalloc/cmpctmalloc.c)
