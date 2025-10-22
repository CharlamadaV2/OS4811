package main

import(
    "fmt"
    "sync/atomic"
)

type pointer_t struct{ //name first type after
    ptr *node_t
    count uint64 //Init as 0
}

type node_t struct{
    value int64 //Init as 0
    next pointer_t
}

type queue_t struct{
    Head pointer_t
    Tail pointer_t
}

func initialize(Q *queue_t) {
    node := &node_t{ //Allocate a free node
        //Make it the only node in the linked list
        next: pointer_t{},
    } 

    //Both head and tail point to node
    Q.Head = pointer_t{ptr: node}
    Q.Tail = pointer_t{ptr: node}

}

func enqueue(Q *queue_t, value int64){
    // Allocate a new node from the free list
    node := &node_t{
        // Copy enqueued value into node
        value: value,
        // Set next pointer of node to NULL
        next: nil,

    }

    // Keep trying until Enqueue is done
    for {
        tail:= Q.Tail //Read Tail.ptr and Tail.count together

        next:= Q.Tail{ptr: next} //Read next ptr and count fields together

        if { //Are tail and next consistent?
            if next.ptr == nil { //Was Tail pointing to the last node?
                if atomic.CompareAndSwapPointer(&tailNode.next, next, unsafe.Pointer(node)) { // CAS(&tail.ptr–>next, next, <node, next.count+1>) # Try to link node at the end of the linked list
                    break // Enqueue is done. Exit loop
                }
            } else {
                // CAS(&Q–>Tail, tail, <next.ptr, tail.count+1>) # Try to swing Tail to the next node
            }
        }
    }
    atomic.CompareAndSwapPointer //CAS(&Q–>Tail, tail, <node, tail.count+1>)    # Enqueue is done. Try to swing Tail to the inserted node

}

func dequeue(){
    for {// loop
    //     head = Q-> Head
    //     tail = Q -> Tail
    //     next = head->next
    //     if head == Q->Head
    //         if head.ptr == tail.ptr
    //             if next.ptr == NULL
    //                 return FALSE
    //             endif
    //             CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
    //         else
    //             #Read value before CAS, otherwise another dequeue might free the next node
    //             *pvalue = next.ptr -> value
    //             if CAS(&Q -> Head, head, <next.ptr, head.count+1>)
    //                 break
    //             endif
    //         endif
    //     endif
    }// endloop
    //free(head.ptr)
    return TRUE
}