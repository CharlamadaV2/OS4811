#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define max_size 5 //Will be used to define the amount of incrimentations

	// This program will computer the pipe process of consumer and producer, but using threads
	// Initially we had two processors, one generating numbers, one taking numbers
	// We only have to print 5 numbers, 2 threads,
void* thread(){

}

	// 2 Threads will generate every other number (less than equal to 5) from their given start number
		// Incriment from start number (0 + 1, or 1 + 1)
		// Alternatively increment from previous number
		// Attempt Lock

		// If can't lock 
			//Enter queue
			//Wait

		// If lock (will be accessing the critical section)
			//Print curr number
			printf();
			//Send number to consumer
			write();
				
			//Then unlock

		// Loop while curr num is <= 5

		// Delete process

	// As a result it will prevent wait time on generating the number
	// Will only wait on the consumer printing the number

	//Consumer (critical section) will be taking inputs from either threads 
	// depending on who is currently accesing the critical section.	
	// Read request
	read();
	// Print number taken
	printf();

	// Subsequently, we will need to set up a queue for locking access.
		//Thread 1 PID < Thread 2, therefore first contact

		// The other thread will be in a ready state meaning it will get access as soon as consumer is unlocked

int main(){
	pthread_t thread1, thread2; // Thread handles
    int id1 = 1, id2 = 2;


}