# Replay Engine

## The Problem

Multithreading is inherently non-deterministic. Thread scheduling, CPU core assignment, and OS-level preemption all introduce unpredictable ordering into concurrent operations. For systems that require exact replay (financial trading engines, distributed databases, audit systems), this is unacceptable.

A simple test proves the point. Two threads incrementing a shared counter 10 times each should always produce 20. Run it a million times:

```cpp
long long counter = 0;

void worker() {
    for (int i = 0; i < 10; i++) {
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
```

| Threads | Tests Run | Pass Rate |
|---------|-----------|-----------|
| 1 | 1,000,000 | 100.00% |
| 2 | 1,000,000 | 66.49% |
| 3 | 1,000,000 | 64.58% |
| 4 | 1,000,000 | 61.39% |
| 5 | 1,000,000 | 55.86% |
| 10 | 1,000,000 | 24.24% |

In this test, the pass rate falls as more threads contend for the shared counter.

## The Solution

A C++20 event replay engine that assigns sequence IDs before processing and checks replay results across thread-count configurations.

After implementing the engine, the same class of operations produces:

| Threads | Events | Checksum Match | Pass Rate |
|---------|--------|----------------|-----------|
| 1 | 10,000 | Yes | 100% |
| 2 | 10,000 | Yes | 100% |
| 4 | 10,000 | Yes | 100% |
| 8 | 10,000 | Yes | 100% |

The tested configurations produced the same checksum and result order across repeated runs.

---

## C++ Engine

### Architecture

The engine uses a 3-stage pipeline with lock-free queues connecting each stage:

```
Producer Thread        Processor Thread        Collector Thread
     |                      |                       |
  [Input]  ------>  [Work Queue]  ------>  [Output Queue]  ------> Results
  LFQueue            LFQueue               LFQueue
```

**Producer** reads raw events from the input queue, assigns monotonically increasing sequence IDs via `atomic::fetch_add`, and pushes `SequencedEvent` structs into the work queue.

**Processor** dequeues sequenced events, computes state values based on event type, applies atomic state updates (price, volume, order count, checksum), and pushes `ProcessedResult` structs into the output queue. Processing is sequential within this thread, which is what guarantees deterministic ordering without spin-wait barriers or lock contention.

**Collector** drains the output queue into a result vector for final consumption.

All three threads run concurrently. Pipeline parallelism provides throughput while sequential processing within each stage provides determinism.

### Lock-Free Queue (Michael-Scott MPMC)

The `LFQueue<T>` is a Michael-Scott queue implementation using atomic compare-and-swap:

- Dummy node sentinel to avoid empty-queue edge cases
- `enqueue`: CAS on `tail->next`, then advance `tail` with a second CAS
- `dequeue`: CAS on `head` to swing past the dummy to the real first node
- Helping mechanism in `enqueue` (advancing a lagging tail pointer) to maintain lock-freedom

The queue uses atomic compare-and-swap operations rather than mutex-based coordination. Its lock-free progress guarantee does not imply that every operation completes in a fixed amount of time.

### Memory Layout

- `alignas(64)` on `SequencedEvent` and `ProcessedResult` to prevent false sharing across cache lines
- `alignas(64)` on all atomic state variables (`globalPrice`, `globalVolume`, `activeOrders`, `stateChecksum`)
- Atomic state updates use `memory_order_relaxed` where ordering guarantees aren't needed, `memory_order_release`/`memory_order_acquire` at synchronization boundaries

### Deterministic Checksumming

State integrity is verified through XOR-based checksumming applied atomically after each event:

```cpp
uint64_t hash = result.sequenceId * 2654435761ULL;  // Knuth multiplicative hash
hash ^= valueBits;                                    // IEEE 754 double bits
hash ^= result.orderId * 40503ULL;
stateChecksum.fetch_xor(hash, std::memory_order_relaxed);
```

XOR is commutative and associative, so the final checksum is order-independent within the atomic operation while the sequential processing ensures the sequence of XOR applications is deterministic.

### Event Types

Five event types model a financial trading system:

| Type | State Computation |
|------|-------------------|
| NewOrder | `price * quantity` |
| CancelOrder | `-1` (decrement active orders) |
| Trade | `price * quantity * direction` |
| ModifyOrder | `price * quantity` (updated values) |
| QuoteUpdate | `price` |

### Performance

- 1.7M+ events/sec sustained throughput
- Sub-microsecond per-event processing latency
- Bit-identical checksums across 1, 2, 4, and 8 thread configurations
- 10 consecutive runs produce identical state on every execution

### Test Suite

45 tests across 6 modules, 100% pass rate:

| Module | Tests | Coverage |
|--------|-------|----------|
| Replay Determinism | 10 | Bit-identical checksums, sequence ordering, multi-run consistency, varying thread counts |
| Stress | 7 | 50k event throughput, max threads, rapid start/stop (20 cycles), concurrent submission, burst traffic |
| Memory | 7 | Cache alignment verification, leak detection (50 cycles), large batch (50k), queue cleanup |
| Metrics | 6 | Event count accuracy, latency tracking, throughput measurement, producer/consumer parity |
| Statistics | 6 | Checksum distribution, latency distribution, worker load balance, throughput scaling |
| Fault Injection | 9 | Graceful shutdown, early stop, double start/stop, zero-price events, max-value events, destructor safety |

### Build

```bash
mkdir build && cd build
cmake ..
make
./replay_engine
```

Requires C++20. Tested on macOS (Apple Clang) and Linux (GCC 12+).

---

## Web Demo

An interactive browser-based simulation of the engine, built for portfolio demonstration.

The web version is a TypeScript port that replicates the engine's pipeline architecture and deterministic behavior. It does not use lock-free queues or real threads. JavaScript is single-threaded, so the concurrency model is simulated: `requestAnimationFrame` drives batches through the 3-stage pipeline one tick at a time, and worker IDs are assigned via round-robin modulo for visualization purposes.

What it does preserve from the C++ engine:
- 3-stage pipeline flow (produce, process, collect) with batch processing
- Monotonic sequence ID assignment
- Identical `processEvent` state value computation per event type
- Identical XOR-based checksum algorithm (bit-identical results between TS and C++)
- Determinism verification across multiple configurations

### Features

- Configurable event count, worker count, batch size, and event types
- Animated pipeline visualization showing events flowing through each stage
- Live metrics panel (throughput, latency, checksum)
- Worker distribution chart and latency histogram
- Determinism verification matrix (runs multiple configs side-by-side)
- 20-test suite runnable in the browser

### Stack

React, TypeScript, Vite, Tailwind CSS

### Run locally

```bash
cd web
npm install
npm run dev
```

### Deploy

Configured for GitHub Pages via GitHub Actions. Push to `main` and it auto-deploys.
