# Design
Two-Lock Queue
- Uses two sync.Mutex locks — one for head, one for tail.
- Operations on head and tail can occur concurrently
- Based on Figure 29.9 from *Operating Systems: Three Easy Pieces (OSTEP)*.
- Simple, safe, performs well under low- to moderate contention.

Michael–Scott Lock-Free Queue
- Uses Go atomic.Pointer and CAS loops to avoid locks
- Threads “help” each other by advancing the tail pointer when it lags behind.
- Based on Michael & Scott (1996) — *“Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms”*.
- Scales better under high contention, but more complex with higher constant overhead.

# Benchmark Design
- Tests both queue types under multiple producer/consumer counts.
- Measures enqueue/dequeue total runtime.
- Can adjust workload size and artificial delay to simulate different contention levels (adjust parameters directly inside main.go).
- The structure of the benchmark was designed with assistance from ChatGPT. Group implemented the design, organization and syntax.

# How to Run
```bash
go run .
