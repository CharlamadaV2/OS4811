#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_GEN 5 // Amount of numbers to generate

// Shared variable
int curr_num = 0; // Initialize to avoid undefined behavior
int data_ready = 0; // Flag to indicate data is ready for consumer

// Mutex and condition variables
pthread_mutex_t mutex;
pthread_cond_t producer_cond, consumer_cond;

void *producer_t(void *arg) {
    for (int i = 1; i <= NUM_GEN; i++) {
        pthread_mutex_lock(&mutex);

        // Wait if consumer hasn’t processed the previous number
        while (data_ready) {
            pthread_cond_wait(&producer_cond, &mutex);
        }

        // Produce number
        curr_num = i;
        printf("Producer: %d\n", curr_num);

        // Signal consumer that data is ready
        data_ready = 1;
        pthread_cond_signal(&consumer_cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *consumer_t(void *arg) {
    for (int i = 0; i < NUM_GEN; i++) {
        pthread_mutex_lock(&mutex);

        // Wait if no new data is ready
        while (!data_ready) {
            pthread_cond_wait(&consumer_cond, &mutex);
        }

        // Consume number
        printf("Consumer: %d\n", curr_num);

        // Signal producer that data has been consumed
        data_ready = 0;
        pthread_cond_signal(&producer_cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t producer, consumer;

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return 1;
    }
    if (pthread_cond_init(&producer_cond, NULL) != 0 ||
        pthread_cond_init(&consumer_cond, NULL) != 0) {
        perror("Condition variable initialization failed");
        return 1;
    }

    // Create threads
    if (pthread_create(&producer, NULL, producer_t, NULL) != 0 ||
        pthread_create(&consumer, NULL, consumer_t, NULL) != 0) {
        perror("Thread creation failed");
        return 1;
    }

    // Wait for threads to finish
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&producer_cond);
    pthread_cond_destroy(&consumer_cond);

    return 0;
}