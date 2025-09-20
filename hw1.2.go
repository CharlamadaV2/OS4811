// This program will computer the pipe process of consumer and producer, but using threads
// Initially we had two processors, one generating numbers, one taking numbers
// We only have to print 5 numbers, 2 threads,
// 2 Threads will generate number from 1 to 5
	// Generate curr number 
	// Try to Lock
	// If can't lock 
		//Next number
		//Spin
	// If lock
		//Print curr number
		//Send number to consumer
		//Then unlock
	// Generate next number
	// Loop until reach 5

// As a result it will prevent wait time on generating the number
// Will only wait on the consumer printing the number

//Consumer will be taking inputs from either threads depending on who is currenlt locked.