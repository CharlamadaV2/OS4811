// classic two-lock concurrent queue
// code adapted from figure 29.9 in *Operating Systems: Three Easy Pieces*
// (Remzi & Andrea Arpaci-Dusseau, University of Wisconsin – Madison, 2024 edition).
package main

import "sync"

// node represents one element in the queue
type node struct {
    val  int
    next *node
}

// TwoLockQueue uses two mutexes: one for head (dequeue) and one for tail (enqueue)
type TwoLockQueue struct {
    headLock sync.Mutex
    tailLock sync.Mutex
    head     *node // dummy node
    tail     *node
}

// Constructor function
func NewTwoLockQueue() *TwoLockQueue {
    dummy := &node{} // dummy helps avoid special cases
    return &TwoLockQueue{head: dummy, tail: dummy}
}

func (q *TwoLockQueue) Enqueue(x int) {
    n := &node{val: x}

    q.tailLock.Lock()        // lock tail so only one enqueue at a time
    q.tail.next = n          // link new node
    q.tail = n               // move tail forward
    q.tailLock.Unlock()      // unlock so others can enqueue
}

func (q *TwoLockQueue) Dequeue() (int, bool) {
    q.headLock.Lock()        // lock head so only one dequeue at a time
    defer q.headLock.Unlock()

    first := q.head.next     // first real node
    if first == nil {
        return 0, false      // empty queue
    }

    q.head = first           // move dummy forward
    return first.val, true
}
