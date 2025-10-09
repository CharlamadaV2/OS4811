//Notes from the book, analogously we could imagine the machanism for the system we will be as 
// Target, how are customers segregated based on the items they have
// Customers with less items generally get to go first because they take less time
// While customers who have more items will be set a side to a parallel line for only multiple items

// In the book they used SJF, Short job first
// The concept is to reduce the avg turn around time
// Another one is context switching 
// Another is STCF, Shortest time completion first

// You can use multiple schedulers SJF -> STCF 

// To satisfy response time, you can use round robin
// Round robin uses time slicing/scheduling quantum

// Knowing these two types of schedulers, we have to figure out the trade off 
// between response time and turn around time prioritization

// When dealing with I/O, we can use the overlap method
// This means we have two processes, A and B
// Process A has 2 jobs, cpu time and I/O
// Process B has 1 job, cpu time, but that time is longer than A
// What we do in this scenario is apply SJF for cpu time share
// A runs then before I/O, B runs concurrently with the I/O. 
// This works because the jobs are independently bounded

// W/O knowing the length of job we use a multi-level feedback queue
// Three queues of priority: high, med, low
// MLFQ wants to balance between reposonse time (iteractive) and turn around (speed)

// Two basic rules, If Prior(A) > B, A runs
// If A = B, run in RR

// MLFQ will use history of the job to predict future behavior
// Implemention for changing priority requires job's allotment
// The allotment is time limit jobs can spend at certain levels

// New rules
// When new job, it is at top prio
// If job uses allotment, prio decreases
// If job gives up CPU before allotment it stays at prio

//Starvation will happen if there are too many small running jobs
// Long running jobs won't ever get prio

//New rule: after period S all jobs are top prio

//Rule 4: once time uses allotment at level regardless of cpu give up
// The Solaris MLFQ implementation, lower priority, longer quanta

// Nice can be used to manually increase or decrease job prio
