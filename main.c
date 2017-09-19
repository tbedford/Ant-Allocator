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
