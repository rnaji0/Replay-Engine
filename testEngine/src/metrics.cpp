#include "metrics.hpp"
#include <sstream>
#include <chrono>
#include <iomanip>

MetricsTests::MetricsTests() : TestModule("MetricsTests") {}

void MetricsTests::registerTests() {
    addTest("EventCountAccuracy", [this]() { return testEventCountAccuracy(); });
    addTest("LatencyTracking", [this]() { return testLatencyTracking(); });
    addTest("ThroughputMeasurement", [this]() { return testThroughputMeasurement(); });
    addTest("StatsConsistency", [this]() { return testStatsConsistency(); });
    addTest("ProducerConsumerParity", [this]() { return testProducerConsumerParity(); });
    addTest("LatencyBounds", [this]() { return testLatencyBounds(); });
}

TestResult MetricsTests::testEventCountAccuracy() {
    const std::string name = "EventCountAccuracy";
    const uint64_t count = 10000;

    ReplayEngine engine(4);
    for (uint64_t i = 0; i < count; ++i) {
        engine.submitEvent(Event(i, i, 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    const auto& stats = engine.getStats();
    if (stats.eventsProduced.load() != count) {
        return fail(name, "produced count: " + std::to_string(stats.eventsProduced.load()));
    }
    if (stats.eventsProcessed.load() != count) {
        return fail(name, "processed count: " + std::to_string(stats.eventsProcessed.load()));
    }
    if (stats.eventsCollected.load() != count) {
        return fail(name, "collected count: " + std::to_string(stats.eventsCollected.load()));
    }
    return pass(name, "all counts match: " + std::to_string(count));
}

TestResult MetricsTests::testLatencyTracking() {
    const std::string name = "LatencyTracking";

    ReplayEngine engine(2);
    for (int i = 0; i < 5000; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    const auto& stats = engine.getStats();
    uint64_t minLat = stats.minLatencyNs.load();
    uint64_t maxLat = stats.maxLatencyNs.load();
    uint64_t totalLat = stats.totalLatencyNs.load();
    uint64_t processed = stats.eventsProcessed.load();

    if (minLat == UINT64_MAX || maxLat == 0) {
        return fail(name, "latency values not updated");
    }
    if (minLat > maxLat) {
        return fail(name, "min > max latency");
    }

    double avgLat = static_cast<double>(totalLat) / processed;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    oss << "min=" << minLat << "ns avg=" << avgLat << "ns max=" << maxLat << "ns";
    return pass(name, oss.str());
}

TestResult MetricsTests::testThroughputMeasurement() {
    const std::string name = "ThroughputMeasurement";
    const uint64_t count = 10000;

    ReplayEngine engine(4);
    std::vector<Event> events;
    events.reserve(count);
    for (uint64_t i = 0; i < count; ++i) {
        events.emplace_back(i, i, 50.0, uint32_t(10), 'B');
    }
    engine.submitEvents(events);

    auto wallStart = std::chrono::steady_clock::now();
    engine.start();
    engine.waitForCompletion();
    auto wallEnd = std::chrono::steady_clock::now();

    double wallSec = std::chrono::duration<double>(wallEnd - wallStart).count();
    double throughput = count / wallSec;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    oss << throughput << " events/sec";
    return pass(name, oss.str());
}

TestResult MetricsTests::testStatsConsistency() {
    const std::string name = "StatsConsistency";

    ReplayEngine engine(3);
    for (int i = 0; i < 2000; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    const auto& stats = engine.getStats();
    uint64_t produced = stats.eventsProduced.load();
    uint64_t processed = stats.eventsProcessed.load();
    uint64_t collected = stats.eventsCollected.load();

    if (produced != processed) {
        return fail(name, "produced != processed");
    }
    if (processed != collected) {
        return fail(name, "processed != collected");
    }
    return pass(name, "produced == processed == collected == " + std::to_string(produced));
}

TestResult MetricsTests::testProducerConsumerParity() {
    const std::string name = "ProducerConsumerParity";

    for (uint32_t threads : {1, 2, 4}) {
        ReplayEngine engine(threads);
        for (int i = 0; i < 3000; ++i) {
            engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
        }
        engine.start();
        engine.waitForCompletion();

        const auto& stats = engine.getStats();
        if (stats.eventsProduced.load() != stats.eventsProcessed.load()) {
            return fail(name, "parity mismatch with " + std::to_string(threads) + " threads");
        }
    }
    return pass(name, "producer/consumer parity across 1, 2, 4 threads");
}

TestResult MetricsTests::testLatencyBounds() {
    const std::string name = "LatencyBounds";

    ReplayEngine engine(2);
    for (int i = 0; i < 1000; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    const auto& stats = engine.getStats();
    uint64_t avgNs = stats.totalLatencyNs.load() / stats.eventsProcessed.load();

    // per-event processing should be under 10ms in a sane environment
    if (avgNs > 10'000'000) {
        return fail(name, "avg latency > 10ms: " + std::to_string(avgNs) + "ns");
    }
    return pass(name, "avg latency: " + std::to_string(avgNs) + "ns");
}
