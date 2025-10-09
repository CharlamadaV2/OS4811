// Ideally, it should alternate between two different lock implementations
// and a standard sync.Mutex to compare their performance under contention.
// This code benchmarks TicketLock and CASSpinLock implementations.

package main

import (
	"fmt"     // Used for printing benchmark results
	"runtime" // Used for Gosched() to yield in spin loops
	"sync"    // Used to implement the locks
	"sync/atomic" // Compare and Swap operations, atomic operations
	"time"    // Used for measuring time
)

// TicketLock implements a ticket lock
type TicketLock struct {
	nowServing uint64
	nextTicket uint64
}

func (tl *TicketLock) Lock() {
	fmt.Printf("nextTicket: %d\n",atomic.LoadUint64(&tl.nextTicket))
	myTicket := atomic.AddUint64(&tl.nextTicket, 1) - 1
	fmt.Printf("myTicket: %d\n", myTicket)
	for atomic.LoadUint64(&tl.nowServing) != myTicket {
		runtime.Gosched() // Yield to other goroutines
		
	}
}

func (tl *TicketLock) Unlock() {
	atomic.AddUint64(&tl.nowServing, 1)
}

// CASSpinLock implements a compare-and-swap spin lock
type CASSpinLock struct {
	locked uint32
}

func (cas *CASSpinLock) Lock() {
	for !atomic.CompareAndSwapUint32(&cas.locked, 0, 1) {
		runtime.Gosched() // Yield to other goroutines
	}
}

func (cas *CASSpinLock) Unlock() {
	atomic.StoreUint32(&cas.locked, 0)
}


func benchmarkLock(name string, lock interface{}, numGoroutines int, iterations int) float64 {
	var wg sync.WaitGroup
	start := time.Now()
	counter := int64(0) // Non-atomic, protected by lock
	
	for i := 0; i < numGoroutines; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for j := 0; j < iterations; j++ {
				switch l := lock.(type) {
				case *TicketLock:
					l.Lock()
					counter++ // Simple increment
					// Simulate tiny bit of work
					_ = counter * 2
					l.Unlock()
					
				case *CASSpinLock:
					l.Lock()
					counter++
					_ = counter * 2
					l.Unlock()
					
				case *sync.Mutex:
					l.Lock()
					counter++
					_ = counter * 2
					l.Unlock()
				}
			}
		}()
	}

	wg.Wait()
	duration := time.Since(start).Seconds()
	fmt.Printf("%s (G: %d, Iter: %d): %.4fs\n",
		name, numGoroutines, iterations, duration)
	return duration * 1e9 / float64(numGoroutines*iterations)
}

func main() {
	// Test configurations
	goroutineCounts := []int{1, 10}
	iterations := 1

	for _, g := range goroutineCounts {
		fmt.Printf("\nTesting with %d goroutines\n", g)
		
		// Ticket Lock
		ticketLock := &TicketLock{}
		benchmarkLock("TicketLock", ticketLock, g, iterations)
		
		// CAS Spin Lock
		casLock := &CASSpinLock{}
		benchmarkLock("CASSpinLock", casLock, g, iterations)
	}
}