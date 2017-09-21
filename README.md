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
nodes that manage the blocks memory.

The following diagram shows this:

![Heap Memory Allocator](./images/Memory_allocator_1.png)

Nodes are for managing blocks, blocks are the chunks of memory an app
will actually use.

TODO: I need to redo the diagrams:

* Nodes - used for managing linked list
* Mem - the memory the application will actually use
* Heap - the data structure used to manage the whole thing
* Block - The mem plus the node (containing the metadata)

The nodes contain a pointer to the mem, the size of the mem,
whether the mem is used or not, and a couple of pointers to allow
the nodes to be hooked into a doubly-linked list.

The linked-list is used when searching for a suitable block to
allocate.

A doubly-linked list seems a bit complex, but there are points where
you will need to check the previous and next blocks to see if they are
free and therefore can be coalesced with the current block. For
example, if you have [free, used, free] blocks and then you free the
used block you have [free, free, free]. These blocks can be coalesced
into one free block.

Note that, in the diagram, a `free()` has meant that there are two
contiguous free blocks (adjacent red blocks) that could be coalesced
into one free block, thus helping to avoid memory fragmentation.

TODO: look at when you find a block but it's a lot bigger than
required, so you have to split the block.

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
don't want the large allocation area ful of small holes. For example
you could end up with say 32KB total free, but only be able to service
a `malloc()` of 4KB. Not good.

![Memory fragmentation](./images/Memory_allocator_3.png)

In the above diagram, there is 70KB free on the heap. However, any
allocation more than 10KB will fail. 

## Implementation

In a real implementation a node (metadata) will be part of a block,
and not separate as there's no `malloc()` available to you to allocate
for the node, because that's what you are implementing.

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
the OS to grab a nice hunk of memory to manage as the
heap. Potentially you could go back cap in hand to the OS at some
point to expand the heap, but you really don't want to be doing that
on every `malloc()`.

You will need something to free up the main heap too, to return memory
ultimately to the system.

## malloc()

Simple case: You walk along the linked-list checking for the first
free block of suitable size. Unless the free block was of exactly the
right size, it would then be split into an allocated block and a new
free block in the list (with its new metadata). This requires you
insert a new node (a free block) into the linked-list. This is
basically the "first fit" approach.

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


## Blocks containing metadata

For the purposes of my implementation a block is a lump that contains
both the node and the memory that the requesting application will
actually use for its own purposes.

This is shown in a modified version of the first diagram:

![Heap Memory Allocator with nodes in block](./images/Memory_allocator_2.png)

Note that it is necessary to allocate larger than the requested memory
size - that is, you need to allocate memory size plus node size.

As you know the location of the node in memory (calculated from ptr
passed to free(), or from walking the list during malloc()), you can
simply add a constant to it (the node size) to find the start point of
the actual allocation. However, a pointer to memory is stored in the
node for convenience.

Remember when implementing `free()` you would take the passed pointer
and subtract the node size to get the start of the metadata.

## Wilderness

TODO

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

