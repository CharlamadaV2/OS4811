typedef int datatype;

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct __pointer_t
{
    lf_node_t *ptr;
    unsigned count
} lf_pointer_t;

typedef struct __node_t
{
    datatype value;
    lf_pointer_t *next
} lf_node_t;

typedef struct __queue_t{
    lf_pointer_t *head;
    lf_pointer_t *tail;
} lf_queue_t;


void lf_initialize(lf_queue_t *q);
void lf_enqueue(lf_queue_t *q, datatype value);
bool lf_dequeue(lf_queue_t *q, datatype *pvalue);


