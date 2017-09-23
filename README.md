# Ant-Allocator

*Currently under heavy construction.*

A very, very simple memory allocator for educational purposes.

Features:

* Minimal design
* Err, that's about it!

## Background

So, in a moment of madness I decided to write my own memory allocator
similar to something like `malloc()`. This was purely for the learning
experience. I also hadn't done any C programming for over 10 years so
decided it was time to get back on the horse again so to speak.

## Overview

After struggling with pen and paper for a while, I realized that it
helps to visualize the allocated memory (used and free) as separate from the
nodes that manage the blocks of memory.

The following diagram shows this:

![Heap Memory Allocator](./images/Memory_allocator_1.png)

Basic definitions of things in my implementation:

* Nodes - used for managing allocated blocks in a doubly linked-list.
* Mem - the memory the application will actually use.
* Heap - the data structure used to manage the whole thing.
* Block - The mem plus the node (containing the metadata).
* Wilderness - the big chunk of free memory after your allocated blocks (if any).

The nodes contain a pointer to the mem, the size of the mem,
whether the mem is used or not, and a couple of pointers to allow
the nodes to be hooked into a doubly-linked list.

The linked-list is used when searching for a suitable block to
allocate.

A doubly-linked list seems a bit complex, but there are points where
you will need to check the previous and next blocks to see if they are
free and therefore can be coalesced with the current block. 

Note that, in the diagram, a `free()` has meant that there are two
contiguous free blocks (adjacent red blocks) that could be coalesced
into one free block, thus helping to avoid memory fragmentation.

As another example, if you have [free, used, free] blocks and then you
free the used block you have [free, free, free]. These blocks can be
coalesced into one free block. You would first coalesce the two on the
left, and then coalesce that with the third block.

TODO: look at when you find a block but it's a lot bigger than
required, so you have to split the block.

## Blocks containing metadata

For the purposes of my implementation a block is a lump that contains
both the node and the memory that the requesting application will
actually use for its own purposes.

This is shown in a modified version of the first diagram:

![Heap Memory Allocator with nodes in block](./images/Memory_allocator_2.png)

Note that it is necessary to allocate larger than the requested memory
size - that is, you need to allocate memory size plus node size.

As you know the location of the node in memory (calculated from ptr
passed to free(), or from walking the list during `malloc()`), you can
simply add a constant to it (the node size) to find the start point of
the actual allocation. However, a pointer to memory is stored in the
node for convenience.

Remember when implementing `free()` you would take the passed pointer
and subtract the node size to get the start of the metadata.

## Memory fragmentation

Memory fragmentation occurs where you have a number of free blocks,
but they are not of sufficient size to service a `malloc()`. You
basically end up with memory that looks like Swiss cheese - lots of
little holes. Block coalescing is one way around this - that's where
you have two free blocks that you meld together.

There are other techniques such as binning where you try to keep small
allocations in one area and larger allocations in another area. This
way you will end up with small holes in the small allocation area,
that are likely to be sufficient to service `malloc()` requests. You
don't want the large allocation area full of small holes. For example
you could end up with say 32KB total free, but only be able to service
a `malloc()` of 4KB. Not good.

![Memory fragmentation](./images/Memory_allocator_3.png)

In the above diagram, there is 70KB free on the heap. However, any
allocation more than 10KB will fail. 

## Heap init

When you initialize the heap you will most likely call down into the
OS via a system call (it used to be something like `brk()` or
`sbrk()`. So why not simply write a `malloc()` that just uses a system
call every time you want to make an allocation? In a word -
efficiency. OS system calls are comparatively slow, and a fairly
complex application might make thousands of calls to allocate
memory. Imagine something like a DBMS or a game making thousands of
little allocations for small buffers, strings, graphics and so
on. Making a system call every time would be horribly inefficient.

However, there is one point at which you can get away with it and
that's when your app or system starts. It makes a one-time call into
the OS to grab a nice chunk of memory to manage as the
heap. Potentially you could go back cap in hand to the OS at some
point to expand the heap, but you really don't want to be doing that
on every `malloc()`.

You will need something to free up the main heap too, to return memory
to the system.

## malloc()

Simple case: You walk along the linked-list checking for the first
free block of suitable size. Unless the free block was of exactly the
right size, it would then be split into an allocated block and a new
free block in the list (with its new metadata). This requires you
insert a new node (a free block) into the linked-list. This is
basically the "first fit" approach. If you don't find a suitable block
in the list, you can grab some of the wilderness and make one.

TODO: other options / optimizations

## free()

Simple case: You are given the address of the memory to free (which
you got from `malloc()`).

In a real implementation the meta-data (node) associated with
allocated memory would be part of the block itself. So given the
address of the memory to free you can subtract `sizeof(node_t)` from
the pointer and go straight to the correct node in the list. This
gives you immediate access to the metadata. You have enough
information to check adjacent blocks and coalesce blocks as
required. Note you don't need to walk the list as you do with
`malloc().`


## Wilderness

TODO: Add new diagram

Wilderness is the term given to the block of free memory as yet
unallocated.

At the start, when you initialize the heap, you have a list structure
with head and tail both pointing at NULL. You have no nodes in the
system. But you have a big chunk of free memory you grabbed via a
system call. This big block of free memory is your 'wilderness'.

What you could do is at the outset you make the wilderness a big free
block with node data. So you start out with one big free block node -
that is one item in the list. I did start to do this but decided to
separate out the wilderness completely.

So you have this big lump of free memory call the wilderness and no
nodes in your list, and now you do a `malloc()`. What happens is is
some memory to satisfty the `malloc()` request is grabbed from the
wilderness to create the first node in the list. The wilderness
pointer is adjusted so that the wilderness area is shrunk
accordingly. You now have one node in your list which is the first
allocated block. You will make further allocations when free nodes
aren't available, shrinking the wilderness accordingly.

But then you want to free up an already allocated block. This
operation would not affect the wilderness. What would happen is that
you can go directly to the node is the list, and set it to 'free' - no
other changes are required, although in a more complex implementation
you could coalesce any adjacent free blocks to make a bigger free
block. So now you just have a normal node, but it's marked as
free. When you scan along the list to find a suitable free block this
one could possibly be used (if it's big enough) and you don't have to
shrink the wilderness at all.

One advantage of treating the wilderness as something different is it
allows you to have some code that monitors its size and if it gets to
a preset size you can go back to the OS and ask for more memory to
extend the wilderness. You would be performing essentially a system
level `realloc()`. I plan on attempting to implement this feature.

## Splitting block

Block splitting is required when you find a free block in the block list that is larger than required. This is show in the following diagram:

![Block splitting](./images/Block_splitting.png)

Here, block 2 is free, but bigger than required. The idea is to split
this block into a used block and a new free block. Block 2 before the
split can be thought of as consisting of space to be used, and the
'left over' bit labelled as the 'fragment' here. There could be a
potential problem here in that perhaps the fragment is not of a
practical size. For example if your fragment is say eight bytes, and a
block header (node) takes, say, eight bytes, that would not be a
useful fragment. This can be solved by setting a constant, say,
`MIN_FRAG` at a practical value, say 128 bytes, for example. If the
frag size is less than this level then block splitting does not take
place, and you have an allocated block that is slightly larger than
asked for. It is important that the true size of the user memory is
recorded, and not the requested size.

While conceptually straightforward, the implementation of this can be
a little tricky as it requires dealing with a tangle of pointers. The
following diagram attempts to clarify this by showing only the block
headers (nodes) involved:

![Block splitting - nodes](./images/Block_splitting_nodes.png)


So the pointers you will need to manipulate are shown in the following diagram:

![Block splitting - pointers](./images/Block_splitting_pointers.png)

Looking at pseudo-code for the split:

1. P3 = P1->Next 
2. P1->Next = P2 
3. P2->Prev = P1
4. P2->Next = P3
5. P3->Prev = P2

Sizes for user sizes also need to be set accordingly:

![Block splitting - sizes](./images/Block_splitting_sizes.png)


## Coalescing blocks

TODO need diagram

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

