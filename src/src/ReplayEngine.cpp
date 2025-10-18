#include "ReplayEngine.hpp"
#include <algorithm>

ReplayEngine::ReplayEngine(uint32_t numWorkers)
    : numWorkers(numWorkers > 0 ? numWorkers : 1) {}

ReplayEngine::~ReplayEngine() {
    if (running.load()) {
        stop();
    }
}

void ReplayEngine::submitEvent(const Event& event) {
    inputQueue.enqueue(event);
}

void ReplayEngine::submitEvents(const std::vector<Event>& events) {
    for (const auto& e : events) {
        inputQueue.enqueue(e);
    }
}

void ReplayEngine::start() {
    if (running.exchange(true)) return;

    stopRequested.store(false);
    producerDone.store(false);
    processorDone.store(false);
    workerIdCounter.store(0);

    stats.startTime = std::chrono::steady_clock::now();

    producerThread = std::thread(&ReplayEngine::producerLoop, this);
    processorThread = std::thread(&ReplayEngine::processorLoop, this);
    collectorThread = std::thread(&ReplayEngine::collectorLoop, this);
}

void ReplayEngine::stop() {
    stopRequested.store(true);

    if (producerThread.joinable()) producerThread.join();
    if (processorThread.joinable()) processorThread.join();
    if (collectorThread.joinable()) collectorThread.join();

    stats.endTime = std::chrono::steady_clock::now();
    running.store(false);
}

void ReplayEngine::waitForCompletion() {
    stopRequested.store(true, std::memory_order_release);

    while (!producerDone.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    while (!processorDone.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    stop();
}

void ReplayEngine::producerLoop() {
    Event event;
    while (!stopRequested.load(std::memory_order_relaxed)) {
        if (inputQueue.dequeue(event)) {
            uint64_t seq = nextSequenceId.fetch_add(1, std::memory_order_relaxed);
            workQueue.enqueue(SequencedEvent(event, seq));
            stats.eventsProduced.fetch_add(1, std::memory_order_relaxed);
        } else {
            if (stopRequested.load(std::memory_order_relaxed)) break;
            std::this_thread::yield();
        }
    }

    while (inputQueue.dequeue(event)) {
        uint64_t seq = nextSequenceId.fetch_add(1, std::memory_order_relaxed);
        workQueue.enqueue(SequencedEvent(event, seq));
        stats.eventsProduced.fetch_add(1, std::memory_order_relaxed);
    }

    producerDone.store(true, std::memory_order_release);
}

void ReplayEngine::processorLoop() {
    SequencedEvent se;
    while (true) {
        if (workQueue.dequeue(se)) {
            // assign to worker in round-robin for tracking
            uint32_t wid = workerIdCounter.fetch_add(1, std::memory_order_relaxed) % numWorkers;

            auto startTs = std::chrono::steady_clock::now();
            ProcessedResult result = processEvent(se);
            result.workerId = wid;
            applyStateUpdate(result);
            auto endTs = std::chrono::steady_clock::now();

            uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(
                endTs - startTs).count();
            stats.updateLatency(latency);

            outputQueue.enqueue(result);
            stats.eventsProcessed.fetch_add(1, std::memory_order_relaxed);
        } else {
            if (producerDone.load(std::memory_order_acquire) && workQueue.isEmpty()) {
                break;
            }
            std::this_thread::yield();
        }
    }

    processorDone.store(true, std::memory_order_release);
}

ProcessedResult ReplayEngine::processEvent(const SequencedEvent& se) {
    const Event& e = se.event;
    double stateVal = 0.0;

    switch (e.type) {
        case Event::Type::NewOrder:
            stateVal = e.price * e.quantity;
            break;
        case Event::Type::CancelOrder:
            stateVal = -1.0;
            break;
        case Event::Type::Trade:
            stateVal = e.price * e.quantity * (e.action == 'B' ? 1.0 : -1.0);
            break;
        case Event::Type::ModifyOrder:
            stateVal = e.price * e.quantity;
            break;
        case Event::Type::QuoteUpdate:
            stateVal = e.price;
            break;
    }

    auto now = std::chrono::steady_clock::now().time_since_epoch();
    uint64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

    return ProcessedResult(se.sequenceId, e.orderId, stateVal, ts, 0);
}

void ReplayEngine::applyStateUpdate(const ProcessedResult& result) {
    uint64_t valueBits;
    double sv = result.stateValue;
    std::memcpy(&valueBits, &sv, sizeof(valueBits));

    uint64_t hash = result.sequenceId * 2654435761ULL;
    hash ^= valueBits;
    hash ^= result.orderId * 40503ULL;

    stateChecksum.fetch_xor(hash, std::memory_order_relaxed);

    if (result.stateValue >= 0) {
        double oldPrice = globalPrice.load(std::memory_order_relaxed);
        double newPrice;
        do {
            newPrice = oldPrice + result.stateValue;
        } while (!globalPrice.compare_exchange_weak(oldPrice, newPrice, std::memory_order_relaxed));

        if (result.stateValue > 0) {
            activeOrders.fetch_add(1, std::memory_order_relaxed);
        }
    } else {
        activeOrders.fetch_sub(1, std::memory_order_relaxed);
    }

    uint64_t vol = static_cast<uint64_t>(
        result.stateValue > 0 ? result.stateValue : -result.stateValue);
    globalVolume.fetch_add(vol, std::memory_order_relaxed);
}

void ReplayEngine::collectorLoop() {
    ProcessedResult result;
    while (true) {
        if (outputQueue.dequeue(result)) {
            std::lock_guard<std::mutex> lock(resultsMutex);
            collectedResults.push_back(result);
            stats.eventsCollected.fetch_add(1, std::memory_order_relaxed);
        } else {
            if (processorDone.load(std::memory_order_acquire) && outputQueue.isEmpty()) {
                break;
            }
            std::this_thread::yield();
        }
    }
}

std::vector<ProcessedResult> ReplayEngine::getResults() const {
    std::lock_guard<std::mutex> lock(resultsMutex);
    return collectedResults;
}

uint64_t ReplayEngine::getStateChecksum() const {
    return stateChecksum.load(std::memory_order_acquire);
}

const EngineStats& ReplayEngine::getStats() const {
    return stats;
}

bool ReplayEngine::isRunning() const {
    return running.load(std::memory_order_relaxed);
}

uint64_t ReplayEngine::eventsProcessedCount() const {
    return stats.eventsProcessed.load(std::memory_order_relaxed);
}
