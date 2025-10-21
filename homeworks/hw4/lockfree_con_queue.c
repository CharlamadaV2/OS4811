#include <lockfree_con_queue.h>

void initialize(){
    //node = new_node()
    //node -> next.ptr = NULL
    //Q -> Head = Q -> Tail = node

}

void enqueue(){
    //node = new_node()
    //node -> value = value
    //node -> next.ptr = NULL
    /*loop
        tail = Q -> Tail
        next = tail.ptr -> next
        if tail == Q -> Tail
            if next.ptr == NULL
                if CAS(&tail.ptr -> next, next, <node,nextcount+1>)
                    break
                endif
            else
                CAS(&Q -> Tail, tail, <next.ptr, tail.count +1>)
            endif
        endif
    endloop*/
    //CAS (&Q -> Tail, tail, <node, tail.count+1>)

}

bool dequeue(){
    /*loop
        head = Q-> Head
        tail = Q -> Tail
        next = head->next
        if head == Q->Head
            if head.ptr == tail.ptr
                if next.ptr == NULL
                    return FALSE
                endif
                CAS(&Q->Tail, tail, <next.ptr, tail.count+1>)
            else
                #Read value before CAS, otherwise another dequeue might free the next node
                *pvalue = next.ptr -> value
                if CAS(&Q -> Head, head, <next.ptr, head.count+1>)
                    break
                endif
            endif
        endif
    endloop*/
    //free(head.ptr)
    //return TRUE
}