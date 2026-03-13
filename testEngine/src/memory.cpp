#include "memory.hpp"
#include <sstream>
#include <new>

MemoryTests::MemoryTests() : TestModule("MemoryTests") {}

void MemoryTests::registerTests() {
    addTest("CacheAlignment", [this]() { return testCacheAlignment(); });
    addTest("NoLeakOnDestruction", [this]() { return testNoLeakOnDestruction(); });
    addTest("RepeatedAllocDealloc", [this]() { return testRepeatedAllocDealloc(); });
    addTest("LargeEventBatch", [this]() { return testLargeEventBatch(); });
    addTest("QueueCleanup", [this]() { return testQueueCleanup(); });
    addTest("EventSizeCompact", [this]() { return testEventSizeCompact(); });
    addTest("AlignedAtomics", [this]() { return testAlignedAtomics(); });
}

TestResult MemoryTests::testCacheAlignment() {
    const std::string name = "CacheAlignment";

    size_t seAlign = alignof(SequencedEvent);
    size_t prAlign = alignof(ProcessedResult);

    if (seAlign < 64) {
        return fail(name, "SequencedEvent alignment is " + std::to_string(seAlign) + ", expected 64");
    }
    if (prAlign < 64) {
        return fail(name, "ProcessedResult alignment is " + std::to_string(prAlign) + ", expected 64");
    }

    std::ostringstream oss;
    oss << "SequencedEvent=" << seAlign << "B, ProcessedResult=" << prAlign << "B";
    return pass(name, oss.str());
}

TestResult MemoryTests::testNoLeakOnDestruction() {
    const std::string name = "NoLeakOnDestruction";

    for (int i = 0; i < 50; ++i) {
        ReplayEngine engine(2);
        for (int j = 0; j < 1000; ++j) {
            engine.submitEvent(Event(uint64_t(j), uint64_t(j), 10.0, uint32_t(1), 'B'));
        }
        engine.start();
        engine.waitForCompletion();
    }
    return pass(name, "50 engine lifecycles completed without crash");
}

TestResult MemoryTests::testRepeatedAllocDealloc() {
    const std::string name = "RepeatedAllocDealloc";

    for (int i = 0; i < 100; ++i) {
        LFQueue<Event> q;
        for (int j = 0; j < 500; ++j) {
            q.enqueue(Event(uint64_t(j), uint64_t(j), 10.0, uint32_t(1), 'B'));
        }
        Event e;
        while (q.dequeue(e)) {}
    }
    return pass(name, "100 queue create/fill/drain cycles stable");
}

TestResult MemoryTests::testLargeEventBatch() {
    const std::string name = "LargeEventBatch";
    const size_t batchSize = 50000;

    {
        ReplayEngine engine(4);
        std::vector<Event> events;
        events.reserve(batchSize);
        for (size_t i = 0; i < batchSize; ++i) {
            events.emplace_back(uint64_t(i), uint64_t(i), 10.0, uint32_t(1), 'B');
        }
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();

        if (engine.getResults().size() != batchSize) {
            return fail(name, "event loss in large batch");
        }
    }

    std::ostringstream oss;
    oss << batchSize << " events allocated and freed";
    return pass(name, oss.str());
}

TestResult MemoryTests::testQueueCleanup() {
    const std::string name = "QueueCleanup";

    {
        LFQueue<Event> q;
        for (int i = 0; i < 10000; ++i) {
            q.enqueue(Event(uint64_t(i), uint64_t(i), 10.0, uint32_t(1), 'B'));
        }
        // partial drain
        Event e;
        for (int i = 0; i < 5000; ++i) {
            q.dequeue(e);
        }
        // destructor should clean up remaining nodes
    }
    return pass(name, "partial-drain queue destructed cleanly");
}

TestResult MemoryTests::testEventSizeCompact() {
    const std::string name = "EventSizeCompact";

    size_t eventSize = sizeof(Event);
    // Event should be reasonably compact (under 64 bytes)
    if (eventSize > 64) {
        return fail(name, "Event is " + std::to_string(eventSize) + " bytes, expected <= 64");
    }

    std::ostringstream oss;
    oss << "Event=" << eventSize << "B, SequencedEvent=" << sizeof(SequencedEvent)
        << "B, ProcessedResult=" << sizeof(ProcessedResult) << "B";
    return pass(name, oss.str());
}

TestResult MemoryTests::testAlignedAtomics() {
    const std::string name = "AlignedAtomics";

    if (!std::atomic<uint64_t>{}.is_lock_free()) {
        return fail(name, "atomic<uint64_t> is not lock-free on this platform");
    }
    if (!std::atomic<double>{}.is_lock_free()) {
        return fail(name, "atomic<double> is not lock-free on this platform");
    }

    return pass(name, "all critical atomics are lock-free");
}
