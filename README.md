# Ant Allocator

An experimental memory allocator.

See `attempt1` directory for first attempt. Status working - and there
is some limited test code. See README in that directory for full
explanations, details, diagrams, code etc.

I will shortly embark on "attempt 2". The objective will be to
significantly reduce the block header from 40 bytes to something more
reasonable - one pointer and a size_t should be enough. The prev
pointer will have to be worked out when scanning the list from head,
rather than stored in the block.

I also realized that I only need to record a list of free blocks, as
long as the application is able to keep track of the size of the
original allocation.
