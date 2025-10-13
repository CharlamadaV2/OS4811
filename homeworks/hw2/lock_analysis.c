
typedef struct __lock_t {
    int flag;
    queue_t *q;
} lock_t;

void lock_init(lock_t *m) {
    m->flag = 0;
    queue_init(m->q);
    
}

void lock(lock_t *m) {
    if (m->flag == 0) {
    m->flag = 1; // lock is acquired
    } else {
        queue_add(m->q, gettid());
        park();
    }
}

void unlock(lock_t *m) {
    if (queue_empty(m->q))
    m->flag = 0; // let go of lock; no one wants it
    else
    unpark(queue_remove(m->q)); 
}

// This is the new algorithm without any of the guard checks,
// and it is incorrect. Thread 1 triggered a lock(m) upon execution; 
// however, there is no unlock(m) throughout the rest of the code. 

// queue_add(m->q, gettid()); -> queue = T1
// park(); -> T1 is suspended
// T2 is nowhere to be seen

// Thus, the first thread is stuck, and the system makes no progress. The property that is violated by this code is liveliness. Due to the first thread being suspended and not triggering the second thread, the system ends and can no longer continue.
