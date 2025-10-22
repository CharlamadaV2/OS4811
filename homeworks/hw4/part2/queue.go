// queue.go
package main

// Queue interface used for both implementations
type Queue interface {
    Enqueue(x int)      // add value
    Dequeue() (int, bool) // remove and return value (false if empty)
}
