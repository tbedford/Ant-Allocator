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
