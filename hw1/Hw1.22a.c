include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <time.h>

#define NUM_GEN 1000000 // Amount of numbers to generate

uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

int main() {
    int pipe1[2]; // Pipe for producer to consumer (numbers)
    int pipe2[2]; // Pipe for consumer to producer (acknowledgments)
    pid_t pid;
    int buffer;

    // Create two pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // Create child process
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child (Consumer)
        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2

        // Read and process numbers
        for (int i = 0; i < NUM_GEN; i++) {
            // Read number from pipe1
            if (read(pipe1[0], &buffer, sizeof(int)) == -1) {
                perror("read failed");
                exit(EXIT_FAILURE);
            }
            printf("Consumer: %d\n", buffer);
            fflush(stdout); // Ensure immediate output

            // Send acknowledgment to parent
            int ack = 1;
            if (write(pipe2[1], &ack, sizeof(int)) == -1) {
                perror("write failed");
                exit(EXIT_FAILURE);
            }
        }

        close(pipe1[0]); // Close read end of pipe1
        close(pipe2[1]); // Close write end of pipe2
        exit(EXIT_SUCCESS);
    } else { // Parent (Producer)
        close(pipe1[0]); // Close read end of pipe1
        close(pipe2[1]); // Close write end of pipe2
        
        uint64_t start = get_time_ns();
        // Generate and send numbers
        for (int i = 1; i <= NUM_GEN; i++) {
            printf("Producer: %d\n", i);
            fflush(stdout); // Ensure immediate output

            // Write number to pipe1
            if (write(pipe1[1], &i, sizeof(int)) == -1) {
                perror("write failed");
                exit(EXIT_FAILURE);
            }

            // Wait for acknowledgment from child
            int ack;
            if (read(pipe2[0], &ack, sizeof(int)) == -1) {
                perror("read failed");
                exit(EXIT_FAILURE);
            }
        }

        uint64_t end = get_time_ns();

        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2
        wait(NULL); // Wait for child to finish
        printf("Execution time: %.3f ms\n", (end - start) / 1e6);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
