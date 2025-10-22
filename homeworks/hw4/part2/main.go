// main.go
// design of the benchmark (multi-producer, multi-consumer with adjustable workloads)
// was assisted by ChatGPT for structure and clarity. logic was then reviewed, simplified, and customized
package main

import (
    "fmt"
    "runtime"
    "sync"
    "time"
)

func main() {
    runBenchmark("Two-Lock", NewTwoLockQueue())
    runBenchmark("Lock-Free", NewMSQueue())
}

func runBenchmark(name string, q Queue) {
    numProducers := 4
    numConsumers := 4
    totalItems := 100000

    var wg sync.WaitGroup
    start := time.Now()

    // Producers
    for p := 0; p < numProducers; p++ {
        wg.Add(1)
        go func(p int) {
            defer wg.Done()
            for i := 0; i < totalItems/numProducers; i++ {
                q.Enqueue(i)
                if i%100 == 0 {
                    runtime.Gosched()
                }
            }
        }(p)
    }

    // Consumers
    for c := 0; c < numConsumers; c++ {
        wg.Add(1)
        go func() {
            defer wg.Done()
            for {
                if _, ok := q.Dequeue(); !ok {
                    // small pause to let producers fill queue
                    time.Sleep(100 * time.Microsecond)
                    if time.Since(start) > 2*time.Second {
                        return
                    }
                }
            }
        }()
    }

    wg.Wait()
    elapsed := time.Since(start)
    fmt.Printf("%s queue took: %v\n", name, elapsed)
}
