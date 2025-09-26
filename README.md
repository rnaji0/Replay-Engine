# Replay-Engine: Deterministic Multi-Threaded Event Replay Engine

## Overview
Replay-Engine is a **high-performance, deterministic, multi-threaded event replay engine** written in C++.  
It is designed to process millions of time-ordered events per second with bit-identical replay guarantees, regardless of thread scheduling or hardware concurrency.


## Why?
Modern high-performance systems like financial trading engines and real-time distributed databases require deterministic correctness replays, ultra-low latency, and concurrency at scale.


## How?
This can be achieved through lock-free cached data structures, deterministic scheduling across threads, and NUMA-aware memory management.


## Architecture

### Threads
A Producer Thread reads events from file/log, assigns sequence numbers, and pushes them into a lock-free input queue.  
Worker Threads concurrently consume events, process them deterministically (based on sequence numbers or thread ownership), and update shared state using atomics or thread-local buffers.  
A Metrics Thread collects processed results from an output queue, measuring latency, throughput, and memory usage with minimal interference.  

### Memory & Performance Design
Preallocated event pools to eliminate runtime allocation.  
Cache-aligned data structures to prevent false sharing.  
Thread-local state buffers to reduce contention.  
Atomic updates for deterministic, reproducible state changes.  

