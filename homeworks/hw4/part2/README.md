Two-Lock Queue

Employs two sync.Mutex locks: one for the head and one for the tail.
Allows concurrent operations on the head and tail.
Derived from Figure 29.9 in Operating Systems: Three Easy Pieces (OSTEP).
Simple, thread-safe, and efficient under low to moderate contention.

Michael–Scott Lock-Free Queue

Utilizes Go’s atomic.Pointer and Compare-And-Swap (CAS) loops to eliminate locks.
Threads collaborate by advancing the tail pointer when it falls behind.
Based on the 1996 paper by Michael & Scott, “Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms”.
Offers better scalability under high contention but is more complex with increased constant overhead.

Benchmark Design

Evaluates both queue implementations with varying numbers of producers and consumers.
Measures total runtime for enqueue and dequeue operations.
Supports adjustable workload sizes and artificial delays to simulate different contention levels (configured directly in main.go).
Benchmark structure designed with assistance from ChatGPT, with implementation, organization, and syntax handled by the group.

# How to Run
```bash
go run .
