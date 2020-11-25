#include "types.h"

struct pqueue_node{
        struct proc *proc;
        uint   priority;
        uint   in_use;
        struct pqueue_node* next;
};

struct pqueue_n{
        struct pqueue_node *head;
        struct pqueue_node *tail;
        int size; /* number of nodes in list / processes in queue */
};