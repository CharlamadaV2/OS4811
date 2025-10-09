// Concurrent Linked List: Rewritten (Figure 29.8) 
// improvement using hand-over-hand locking 
// (see Scaling Linked Lists section) 

// Hand over hand has locks for nodes instead of the list
// When traversing the
// list, the code first grabs the next node’s lock and then releases the current
// node’s lock (which inspires the name hand-over-hand).

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "con_list_hoh.h"

// Initialize the list
void hoh_list_init(hoh_list_t *L) {
    L->head = NULL;
}

// Initialize a node
void hoh_node_init(hoh_node_t *node, int key, hoh_node_t *next) {
    node->key = key;
    node->next = next;
    pthread_mutex_init(&node->lock, NULL);
}

// Insert a new node with the given key at the head of the list
void hoh_node_insert(hoh_list_t *L, int key) {
    hoh_node_t *new = malloc(sizeof(hoh_node_t));
    if (new == NULL) {
        perror("malloc");
        return;
    }
    hoh_node_init(new, key,NULL);

    // If list is empty, no need to lock anything yet
    if (L->head == NULL) {
        L->head = new;
        return;
    }

    // Lock the head node
    pthread_mutex_lock(&L->head->lock);
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&new->next->lock);
}

// Lookup a key in the list
int hoh_list_lookup(hoh_list_t *L, int key) {
    if (L->head == NULL) {
        return -1; // Empty list
    }

    // Lock the first node
    hoh_node_t *curr = L->head;
    pthread_mutex_lock(&curr->lock);

    while (curr) {
        if (curr->key == key) {
            pthread_mutex_unlock(&curr->lock);
            return 0; // Found
        }

        // Lock the next node before unlocking the current
        hoh_node_t *next = curr->next;
        if (next) {
            pthread_mutex_lock(&next->lock);
        }
        pthread_mutex_unlock(&curr->lock);
        curr = next;
    }

    return -1; // Not found
}

// Cleanup a node (destroy mutex and free memory)
void hoh_Node_Destroy(hoh_node_t *node) {
    pthread_mutex_destroy(&node->lock);
    free(node);
}

// Cleanup the entire list
void hoh_List_Destroy(hoh_list_t *L) {
    hoh_node_t *curr = L->head;
    while (curr) {
        hoh_node_t *next = curr->next;
        hoh_Node_Destroy(curr);
        curr = next;
    }
    L->head = NULL;
}