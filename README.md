# Replay-Engine: Deterministic Multi-Threaded Event Replay Engine

## Overview
Replay-Engine is a **high-performance, deterministic, multi-threaded event replay engine** written in C++.  
It is designed to process millions of time-ordered events per second with bit-identical replay guarantees, regardless of thread scheduling or hardware concurrency.


## Why?
Modern high-performance systems like financial trading engines and real-time distributed databases require deterministic correctness replays, ultra-low latency, and concurrency at scale.

From our simple test below, we can prove that mulithreading is inherently not determinstic for a multitude of reasons ranging from our CPU to scheduling done by the OS.
```cpp
#include <iostream>
#include <thread>


long long counter = 0;
int testsPassed = 0;
int testsFailed = 0;
int threadCycle = 10;
int testsRan = 1000000;


void worker() {
    for (int i = 0; i < threadCycle; i++) {
        counter++;
        std::this_thread::yield(); 
    }
}

int runTest() {
    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    return counter;
}

void testController() {
     for (int i = 0; i < testsRan; i++) {
        int testResult = runTest();
        if (testResult != threadCycle * 2) {
            std::cout << "Test " << i << " FAILED: counter = " << testResult << std::endl;
            testsFailed++;
        } else {
            std::cout << "Test " << i << " PASSED: counter = " << testResult << std::endl;
            testsPassed++;
        }
    counter = 0;
    }

    std::cout << std::endl;
    std::cout << "Tests Passed: " << testsPassed << std::endl;
    std::cout << "Tests Failed: " << testsFailed << std::endl;
    std::cout << "Pass Rate: " << (testsPassed / (float)(testsPassed + testsFailed)) * 100 << "%" << std::endl;
}


int main() {
    testController();
    return 0;
}
```

Our results show us how each addition of a thread exponentially decreases the pass rate and accuracy which we test for in deterministic operations.

| Threads | Tests Run   | Pass Rate (%) |
|---------|-------------|---------------|
| 1       | 1,000,000   | 100.0000      |
| 2       | 1,000,000   | 66.4875       |
| 3       | 1,000,000   | 64.5759       |
| 4       | 1,000,000   | 61.3893       |
| 5       | 1,000,000   | 55.8582       |
| 10      | 1,000,000   | 24.2407       |



## How?
This can be achieved through lock-free cached data structures, deterministic scheduling across threads, and NUMA-aware memory management.


## Architecture

### Threads
A producer thread reads events from file/log, assigns sequence numbers, and pushes them into a lock-free input queue. Worker Threads concurrently consume events, process them deterministically (based on sequence numbers or thread ownership), and update shared state using atomics or thread-local buffers. A metrics thread collects processed results from an output queue, measuring latency, throughput, and memory usage with minimal interference.  

### Memory & Performance Design
Preallocated event pools to eliminate runtime allocation. Cache-aligned data structures to prevent false sharing. Thread-local state buffers to reduce contention. Atomic updates for deterministic, reproducible state changes.  

