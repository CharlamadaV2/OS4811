// ms_queue.go short for michael scott queue
// lock-free queue based on the Michael and Scott algorithm

package main

import "sync/atomic"

// msNode is like node but its "next" pointer is atomic
type msNode struct {
    val  int
    next atomic.Pointer[msNode]
}

type MSQueue struct {
    head atomic.Pointer[msNode]
    tail atomic.Pointer[msNode]
}

// Constructor
func NewMSQueue() *MSQueue {
    dummy := &msNode{}
    q := &MSQueue{}
    q.head.Store(dummy)
    q.tail.Store(dummy)
    return q
}

func (q *MSQueue) Enqueue(x int) {
    newNode := &msNode{val: x}
    for {
        tail := q.tail.Load()
        next := tail.next.Load()

        if next == nil {
            // try to link new node at the end
            if tail.next.CompareAndSwap(nil, newNode) {
                // try to move tail forward
                q.tail.CompareAndSwap(tail, newNode)
                return
            }
        } else {
            // someone else already enqueued, help move tail
            q.tail.CompareAndSwap(tail, next)
        }
    }
}

func (q *MSQueue) Dequeue() (int, bool) {
    for {
        head := q.head.Load()
        tail := q.tail.Load()
        next := head.next.Load()

        if next == nil {
            return 0, false // empty
        }
        if head == tail {
            // tail is lagging behind, help catch up
            q.tail.CompareAndSwap(tail, next)
            continue
        }
        val := next.val
        if q.head.CompareAndSwap(head, next) {
            return val, true
        }
    }
}
