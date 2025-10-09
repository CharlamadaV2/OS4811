//Implement Concurrent Linked list: Rewritten (Figure 29.8) 
// and an improvement using hand-over-hand locking 
// (see Scaling Linked lists section) and design a benchmarking
// to evaluate the efficiency, under different workloads. 
// You also need to explain why your workloads effectively measure the performance difference.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "con_list.h"

void list_init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

void list_insert(list_t *L, int key) {
    // synchronization not needed
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) {
        perror("malloc");
        return;
    }
    new->key = key;

    // just lock critical section
    pthread_mutex_lock(&L->lock);
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->lock);
}

int list_lookup(list_t *L, int key) {
    int rv = -1;
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    while (curr) {
        if (curr->key == key) {
            rv = 0;
            break;
        }
        curr = curr->next;  
    }
    pthread_mutex_unlock(&L->lock);
    return rv; // now both success and failure
}

void list_destroy(list_t *L) {
    node_t *curr = L->head;
    while (curr) {
        node_t *next = curr->next;
        free(curr);
        curr = next;
    }
    pthread_mutex_destroy(&L->lock);
}



