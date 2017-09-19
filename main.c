#include<stdio.h>
#include<stdlib.h>

typedef unsigned char byte;

typedef enum { false, true } bool;

typedef struct node_s {
    unsigned int id;
    void *block;
    size_t size; // of actual usable block
    bool free;
    struct node_s *next;
    struct node_s *prev;
} node_t;


typedef struct node_list_s {
    node_t *head;
    node_t *tail;
    void *heap; // for freeing up the whole allocated heap when done
} node_list_t;

void heap_init (node_list_t *nl, size_t hs)
{
    // create first node/block which is the whole heap at the start
    node_t *np = malloc(hs);
    np->id = 0x1234; // for heap checking / testing  
    np->block = np + sizeof(node_t);
    np->size = hs - sizeof(node_t);
    np->free = true;
    np->next = NULL;
    np->prev = NULL;

    nl->head = np;
    nl->tail = NULL;
    nl->heap = np; // this is used to free up whole heap at end of program
    
}

void heap_free(void *h)
{
    free(h);
}

void * ant_alloc (size_t size)
{
    // TODO

    return 0;
}


int main (int argc, char **argv)
{
    printf("Ant allocator!\n");
    printf("Size of size_t %lu!\n", sizeof(size_t));

    node_list_t list;
    size_t heap_sz = 8000;
    heap_init(&list, heap_sz); 

    node_t *p = list.head;
    byte *b = (byte *)p->block;
    printf("Block size: %lu\n", p->size);
    printf("Block ID: %X\n", p->id);

    for (int i; i < p->size; i++)
    {
        *b = 0xAA;
        b++;
    }

    byte data = *((byte *)p->block);
    printf("Data: %X\n", data);

    b--;
    data = *((byte *)b);
    printf("Data: %X\n", data);
    
    heap_free (list.heap);
    
    return 0;
}
