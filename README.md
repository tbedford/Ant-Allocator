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
   block header. This is a genius idea that I "borrowed" from
   Xinu. MBed OS seems to do something similar.

4. And yet another issue related to the previous one. When we
   initialize the heap we should make sure that the heap size is a
   multiple of BLOCKHDR_SZ. Otherwise we will again end up with odd
   fragments of memory that could prove problematic (a free block
   that's not big enough to be added to a free list.) But why truncate
   and no round? Well in embedded systems the system memory available
   for heap is dictated by the hardware. There's some kind of fixed
   memory map. So you can't round up, as that may exceed the actual
   hardware memory available. Thus, you have to truncate.
   

Here are the macros that will round up or down:

``` C
#define roundmb(s)  (size_t)((BLOCKHDR_SZ-1) + (size_t)(s)) & (~(BLOCKHDR_SZ-1)) 
#define truncatemb(s) (size_t)( (size_t)(s) & (~(BLOCKHDR_SZ-1)) )
```

The above macros are probably the cleverest bit in the whole allocator
(they are fast too). Sadly they are not my idea! :(

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

There are a few other points I will list here:

1. Remember with free blocks we always need to preserve space for a
   list (block) header.
2. With allocated blocks we never care what's in them. They don't need
   to have a list header in them, but they do need to be at least big
   enough to store one.
3. As long as we always round requests to a multiple of BLOCKHDR_SZ we
   can be certain that a block will always be big enough to store a
   list header. This is because if you always have a block that is
   size multiple of BLOCKHDR_SZ and you subtract (split) off a
   fragment of a multiple of BLOCKHDR_SZ you will always be left with
   a block of a size which is a multiple of BLOCKHDR_SZ.
4. I now store bock size in the block header rather than user memory
   size. Why? Because the whole of the free block will be allocated,
   it is more convenient to think in terms of block sizes. You have
   much less adjustment of pointers too to find block size. When a
   block is allocated we now never track the pointer to the block -
   the app is responsible for that, so internally we can think in
   terms of blocks sizes.
5. When a user requests an allocation size we round and find a block
   of that size. When the user requests that block to be free they
   specify the original "un rounded" size that they have kept track
   of. That means when they deallocate, specifying the ooriginal
   requested size, we need to round *again* so that we actually
   deallocate the correct size of memory. So there are two places we
   round - on allocation and on free.
   
Another neat trick I "borrowed" from Xinu:

Before I was using a fairly complicated list header structure to keep
track of information about the heap and the beginning (and end) of the
list. I referred to this data structure as "the heap" which wasn't a
good name for it - it is really just a list header with additional
info. So, I split the heap related info off into a heap data
structure. I then created a new variable called `freelist` which is of
type `block_t`. This is the list header. It just has a pointer (to the
head of the list) and what would normally the size of the bock is used
to contain the amount of allocated memory. There's a subtle point
here. Having the list header as the same type as blocks in the free
list (i.e. `block_t`) makes the code simpler in a surprising
way. Because this is a singly-linked list (to reduce block header
size) you have to keep trac of the `prev` pointer (which points at the
block before your `rover` is pointer at). This makes the code simpler
because at the start of the list `prev` can just point at the list
header itself. `prev->next` points at the head of the list. There's
none of the finickety special case code for dealing with processing a
block at the start of the list. The code is just simpler. It's a
really subtle point though and I guess is what separates the genius
programmers like Douglas Comer from us mere mortals. Sigh.

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

### Block to be freed

This shows the block to be freed. When you free the block you need to
allow for the block header:

![freeing a block](./images/freeing_a_block.png)

See the code for more details.

### Block splitting

Now that the allocator uses mutliples of BLOCKHDR_SZ splitting a block
becomes fairly simple. Given a free block and a split point the only
thing you really need to do is adjust the size of the free block, and
return a pointer to the allocated block.

### Block coalescing

Block coalescing is somewhat trickier than before. Here's what you
need to do:

1. You are asked to free an allocated block. The app gives you a
   pointer (that it was given) and its orogonal request size (which it
   keeps track of).

2. You have to walk the list (the slight downside of this
   implementation) to figure out where the newly created free block
   will go. Note the free block list is kept in memory order for this
   reason. 
  
3. You walk the list until you find the place in the list (of free
   blocks) where the deallocation pointer dictates. 
   
4. Assuming (for this example) that there is a free block on the left
   and the right then we have the possibility to coalesce all three
   blocks into one big block. Note in this discussion I'm not going to
   talk about special cases such as the block to be deallocated would
   be at the end of the list and so on (see the source code for more
   details). So, you check whether the left block is exactly adjacent
   to the block to be freed and if so coalesce them together simply by
   adjust the size of the block on the left. Now check the right free
   block to see of it is exactly adjacent to this block. If it is
   adjust the size again to encompass this new block. You also need to
   adjust the next pointer to point to the block after the just
   coalesced one (if there is a next block).
   
It's actually easier to figure this out from looking at the code. 

## Concurrency

There a word we haven't mentioned so far and that's concurrency. The
thing about a lot of embdedded OSs is they live in an interrupt driven
world and interrupts can go off at any time. An interrupt usually
invokes an interrupt handler, and this may cause code to run other
processes (code) to handle the aftermath of the interrupt. In other
words a network packet arriving could generate an interrupt that
causing code in the protocol stack to run, and that may need to
allocate memory for buffers or free memory and so on. Putting it
simply we may be in the code of our memory allocator when suddenly we
are off doing something else and we now have a problem. Why? Because
the memory allocator manipulates a linked list, and say adding a new
node in the list takes several operations (copying of pointers) we
can't be sure that we will finish our pointer jiggling before an
interrupt goes off, which would leave our list in a pickle - and this
would result in further mayhem of another process then started
manipaulting the list and was istself interrupted. Very shortly we
would have a crashed system on our hands. So what's the answer? There
are several possible solutions, but looking at embedded operating
systems like ARM Mbed OS and Xinu is pretty apparent they don't take
any chances - they just disable interrupts while manipulating
lists. Problem solved. One minor point - you do need to save the
current interrupt mask before disabling interrupts, so that once you
have manipulated your list you can restore things as they were before
enabling interrupts again. It does seem somewhat archaic that in the
21st century we need to worry about such details, but there you go!

For simplicity I did not add any concurrency-related code to my
handler. So basically this is not a practical system allocator, or
even application-level allocator in a concurrent environment (i.e. the
real world). Sorry about that!

## Heap exhaustion

There's another problem with this (and similar) allocators - there's
no restriction on how much memory a process can grab from the list. In
a practical system you will have multiple processes all vying for
memory. It is possible for one process to dominate and exhaust memory,
preventing all other processes from runnning. One way of dealing with
this is the use of memory pools. With memory pools you have lists of
buffers of fixed size which can be allocated on a per-process
basis. When a process uses up the buffers in its buffer pool it blocks
until memory is freed (by some other process). This prevents one
process deadlocking the rest of the system through un-checked memory
consumption. Buffer pools will be the next project I work on.

## Last words

There are two insights that nassively simplify and improve the code,
making it more robust:

1. If you subtract a multiple of BLOCKHDR_SZ from a multiple of
   BLOCKHDR_SZ the result will be a multiple of BLOCKHDR_SZ. This
   insight leads to making sure the initial heap is truncated to a
   multiple of BLOCKHDR_SZ and that subsequent allocation request
   sizes are rounded to a multiple of BLOCKHDR_SZ. The simplifies the
   code and removes complex code around checking fragment sizes and
   dealing with edge cases.
   
2. Make the list header the same type as the list node (block)
   type. Another subtle insight that simplifies the code. This allows
   for simpler pointer manipulation at the stat of the list. You can
   only really see this once you have implemented such code.
   
These are probably the most valuable insights I will take away from
this project.

## References

* Operating System Design - The Xinu Approach by Douglas Comer (highly recommended)
* [Xinu GitHub](https://github.com/xinu-os/xinu)
* [Xinu web site](http://xinu-os.org/Main_Page)

If you are interested in operating system internals and/or embedded
systems I highly recommend you take a look at Xinu.

I also took a sneaky peak at the ARM MBED OS code:

* [ARM MBED OS on GitHub](https://github.com/ARMmbed/mbed-os)
