#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "con_list.h"
#include "con_list_hoh.h"

// Testing time
clock_t start_t, end_t;
double total_t;

// Structure to pass arguments to the benchmark function
typedef struct {
    list_t* list;
    int iterations;
    int list_length;
} bench_args_t;

// Benchmark function for threads
void* bench_mark(void* args) {
    bench_args_t* b_args = (bench_args_t*)args;
    list_t* L = b_args->list;
    int iterations = b_args->iterations;
    int list_length = b_args->list_length;

    start_t = clock();
    for (int i = 0; i < iterations; i++) {
        list_lookup(L, list_length);
    }
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("Data for Concurrent Linked List; Nodes: %d Seconds: %.6f Iterations: %d Thread: %lu\n",
           list_length, total_t, iterations, pthread_self());

    return NULL; // Return NULL as required by pthread
}

// Benchmark function for threads
void* hoh_bench_mark(void* args) {
    bench_args_t* b_args = (bench_args_t*)args;
    list_t* L = b_args->list;
    int iterations = b_args->iterations;
    int list_length = b_args->list_length;

    start_t = clock();
    for (int i = 0; i < iterations; i++) {
        hoh_list_lookup(L, list_length);
    }
    end_t = clock();
    total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    printf("Data for Concurrent Linked List w/ HOH; Nodes: %d Seconds: %.6f Iterations: %d Thread: %lu\n",
           list_length, total_t, iterations, pthread_self());

    return NULL; // Return NULL as required by pthread
}

int main() {
    int iterations = 1000;
    int nodes[] = {1, 5, 10, 100};
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_nodes = sizeof(nodes) / sizeof(nodes[0]);
    int num_threads = sizeof(thread_counts) / sizeof(thread_counts[0]);

    // Iterate over different node counts
    for (int cur_node = 0; cur_node < num_nodes; cur_node++) {
        // Initialize the list
        list_t List;
        list_init(&List);

        // Fill the list with nodes
        for (int i = 0; i < nodes[cur_node]; i++) {
            list_insert(&List, i);
        }

        // Iterate over different thread counts
        for (int t = 0; t < num_threads; t++) {
            int thread_count = thread_counts[t];
            pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
            if (!threads) {
                perror("malloc");
                return 1;
            }

            bench_args_t args = {&List, iterations, nodes[cur_node]};

            // Create threads
            for (int t_n = 0; t_n < thread_count; t_n++) {
                if (pthread_create(&threads[t_n], NULL, bench_mark, &args) != 0) {
                    perror("pthread_create");
                    free(threads);
                    list_destroy(&List);
                    return 1;
                }
            }

            // Join threads
            for (int t_n = 0; t_n < thread_count; t_n++) {
                if (pthread_join(threads[t_n], NULL) != 0) {
                    perror("pthread_join");
                }
            }

            free(threads);
        }

        list_destroy(&List);
    }

    //// Benchmark for HOH
    // Iterate over different node counts
    for (int cur_node = 0; cur_node < num_nodes; cur_node++) {
        // Initialize the list
        hoh_list_t List;
        hoh_list_init(&List);

        // Fill the list with nodes
        for (int i = 0; i < nodes[cur_node]; i++) {
            hoh_node_insert(&List, i);
        }

        // Iterate over different thread counts
        for (int t = 0; t < num_threads; t++) {
            int thread_count = thread_counts[t];
            pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
            if (!threads) {
                perror("malloc");
                return 1;
            }

            bench_args_t args = {&List, iterations, nodes[cur_node]};

            // Create threads
            for (int t_n = 0; t_n < thread_count; t_n++) {
                if (pthread_create(&threads[t_n], NULL, hoh_bench_mark, &args) != 0) {
                    perror("pthread_create");
                    free(threads);
                    hoh_List_Destroy(&List);
                    return 1;
                }
            }

            // Join threads
            for (int t_n = 0; t_n < thread_count; t_n++) {
                if (pthread_join(threads[t_n], NULL) != 0) {
                    perror("pthread_join");
                }
            }

            free(threads);
        }

        hoh_List_Destroy(&List);
    }
    return 0;
}
    