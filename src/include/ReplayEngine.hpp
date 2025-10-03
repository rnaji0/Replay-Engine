#ifndef REPLAYENGINE_HPP
#define REPLAYENGINE_HPP

#include <atomic>
#include <thread>
#include <vector>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <cstring>
#include "event.hpp"
#include "LFQueue.hpp"

struct alignas(64) SequencedEvent {
    Event event;
    uint64_t sequenceId;

    SequencedEvent() : event(), sequenceId(0) {}
    SequencedEvent(const Event& e, uint64_t seq) : event(e), sequenceId(seq) {}
};

struct alignas(64) ProcessedResult {
    uint64_t sequenceId;
    uint64_t orderId;
    double stateValue;
    uint64_t processedTimestamp;
    uint32_t workerId;

    ProcessedResult()
        : sequenceId(0), orderId(0), stateValue(0.0),
          processedTimestamp(0), workerId(0) {}

    ProcessedResult(uint64_t seq, uint64_t oid, double sv, uint64_t pts, uint32_t wid)
        : sequenceId(seq), orderId(oid), stateValue(sv),
          processedTimestamp(pts), workerId(wid) {}
};

struct EngineStats {
    std::atomic<uint64_t> eventsProduced{0};
    std::atomic<uint64_t> eventsProcessed{0};
    std::atomic<uint64_t> eventsCollected{0};
    std::atomic<uint64_t> totalLatencyNs{0};
    std::atomic<uint64_t> minLatencyNs{UINT64_MAX};
    std::atomic<uint64_t> maxLatencyNs{0};
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;

    void updateLatency(uint64_t latencyNs) {
        totalLatencyNs.fetch_add(latencyNs, std::memory_order_relaxed);
        uint64_t curMin = minLatencyNs.load(std::memory_order_relaxed);
        while (latencyNs < curMin &&
               !minLatencyNs.compare_exchange_weak(curMin, latencyNs, std::memory_order_relaxed));
        uint64_t curMax = maxLatencyNs.load(std::memory_order_relaxed);
        while (latencyNs > curMax &&
               !maxLatencyNs.compare_exchange_weak(curMax, latencyNs, std::memory_order_relaxed));
    }
};

class ReplayEngine {
public:
    ReplayEngine(uint32_t numWorkers = std::thread::hardware_concurrency());
    ~ReplayEngine();

    void submitEvent(const Event& event);
    void submitEvents(const std::vector<Event>& events);

    void start();
    void stop();
    void waitForCompletion();

    std::vector<ProcessedResult> getResults() const;
    uint64_t getStateChecksum() const;
    const EngineStats& getStats() const;

    bool isRunning() const;
    uint64_t eventsProcessedCount() const;

private:
    void producerLoop();
    void processorLoop();
    void collectorLoop();

    ProcessedResult processEvent(const SequencedEvent& se);
    void applyStateUpdate(const ProcessedResult& result);

    LFQueue<Event> inputQueue;
    LFQueue<SequencedEvent> workQueue;
    LFQueue<ProcessedResult> outputQueue;

    std::atomic<uint64_t> nextSequenceId{1};

    alignas(64) std::atomic<double> globalPrice{0.0};
    alignas(64) std::atomic<uint64_t> globalVolume{0};
    alignas(64) std::atomic<uint64_t> activeOrders{0};
    alignas(64) std::atomic<uint64_t> stateChecksum{0};

    uint32_t numWorkers;
    std::thread producerThread;
    std::thread processorThread;
    std::thread collectorThread;

    std::atomic<bool> running{false};
    std::atomic<bool> producerDone{false};
    std::atomic<bool> processorDone{false};
    std::atomic<bool> stopRequested{false};

    std::vector<ProcessedResult> collectedResults;
    mutable std::mutex resultsMutex;

    EngineStats stats;

    // round-robin worker ID assignment for stats tracking
    std::atomic<uint32_t> workerIdCounter{0};
};

#endif
