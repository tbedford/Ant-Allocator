# Ant Allocator

An experimental memory allocator.

See `attempt1` directory for first attempt. Status working - and there
is some limited test code. See README in that directory for full
explanations, details, diagrams, code etc.

## Attempt 2

There were a few issues with attempt 1:

1. The block list contained both free and used blocks. This means walking the list
   for a suitable free block takes longer because the list conatins used blocks.
2. Our block header was big - 40 bytes. It should be possible to reduce this substantially. 
3. The idea of a wilderness complicated things a bit, for not much benefit.

So with attempt 2 the aim is to:

1. Maintain only a list of free blocks.
2. Have a block header that contains only a next ptr and a usermem_sz.
3. Remove the wilderness idea.

What this means is some benefits and some new headaches too:

1. The application needs to maintain a record of both the ptr to
   allocated memory and the size, as we don't keep this information in
   the block list any more for allocated bocks.
2. Walking the list to find a free block should be a lot faster.
3. We reduce the over head by not managing allocated blocks and by
   reducing the block header size.
4. With attempt 1 freeing an allocated block was trivial because you
   had prev and next pointers built in to the block header to help
   figure out whether you could coalesce blocks. With attempt 2 you
   would now have to walk the block list on free to figure out the
   previous and next blocks in the free list.
   
There are some new gotchas too:

1. Not including allocated blocks in the block list leads to an
   interesting problem. You could potentially allocate a block of a
   small size. This is no problem - until that block is freed. At this
   point the block needs to have the block header added so it can be
   added into the free block list. But what if the block's not big
   enough! There'd be no way to add it into the free list. So the dumb
   approach is to just add the bock header size to all
   allocations. The other option is to add the block header size only
   if the allocation is small. So, if our header is 16 bytes, then we
   might add 16 bytes to allocations of less seventeen bytes. So for a
   allocation of one byte, we would add sixteen bytes for a header. It
   hardly seems worth it for one byte though - and we haven't even
   talked about alignment yet either. If you requested seventeen bytes
   you'd not add the header, so when the block was deallocated you'd
   have a free block with a 16 byte header and 1 byte free. Oh dear.
  
2. NOTE: There's a really subtle gotcha that I hadn't thought of
   before. It happens when you find a block in the list that is bigger
   than required. You can grab some of the free block for the
   allocated mem if it's big enough - it has to be big enough to be
   storeable in a link list when it's freed. That's one gotcha. Here's
   another. We are effectively splitting a block here, and we are
   checking the fragment that we are going to use as allocated
   memory. But what about the shard we are leaving in the free list?
   In other words there are two ways to look at this - when you split
   a block they both need to be big enough to contain a block header.
   
3. The solution to problem 2 is to round up block requests to a
   multiple of the block header size. This has the advantage that when
   you split a block (and you are doing that based on a request that's
   rounded up), you are left with a block that is able to contains the
   block header.

4. And yet another issue related to the previous one. When we
   initialize the heap we should make sure that the heap size is a
   mutliple of BLOCKHDR_SZ. Otherwise we will again end up with odd
   fragments of memory that could prove problematic (a free block
   that's not big enough to be added to a free list.)

Here are the macros that will round up or down:

``` C
#define roundmb(s)  (size_t)((BLOCKHDR_SZ-1) + (size_t)(s)) & (~(BLOCKHDR_SZ-1)) 
#define truncatemb(s) (size_t)( (size_t)(s) & (~(BLOCKHDR_SZ-1)) )
```

The above macros are probably the cleverest bit in the whole allocator
(they are fast too). 

and some test code:

``` C
#include <stdio.h>
#include "ant_allocator.h"

int main (int argc, char **argv)
{

    for (int i=0; i<27; i++)
    {
        printf("%d %lu\n", i, roundmb(i));
    }
    
    for (int i=0; i<27; i++)
    {
        printf("%d %lu\n", i, truncatemb(i));
    }

    printf("truncate 8192: %lu\n", truncatemb(8192));
    printf("truncate 3371: %lu\n", truncatemb(3371));
    printf("truncate 2314: %lu\n", truncatemb(2314));

    return 0;
}

```

## Details

### Initializing the list. 

Because there's no wilderness when the heap is created you have a free
block list that consists of one big free block. This is not too bad to
set up.

![Initialized block list](./images/initialized_list.png)

### First allocation

After the first allocation we have the following:

![First allocation](./images/first_allocation.png)

There are two things we need to do:

* a. Recalculate the new size of the free block
* b. Figure out the pointer to the allocated block

a. To calculate the new size of the free block:

    new_size = old_size - requested_size

b. Calculating pointer: `rover` points at the free block. There are a
couple of ways we can calculate this but the following seems the
simplest:

    p = rover + blockhdr_sz + new_size
    
The returned memory object would contain p and requested_size. 

NOTE: Another gotcha. When you add a constant to a block pointer you
will get a problem because C will use pointer arithmetic. For example:
given block_t *p then p + 1 will add 16 bytes, not 1 byte! Watch out!

### After several allocations

Here's the situation after several allocations:

![After several  allocations](./images/several_allocations.png)

4. Block to be freed

This shows the block to be freed. When you free the block you need to
allow for the block header:

![freeing a block](./images/freeing_a_block.png)
