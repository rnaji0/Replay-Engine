#include "statistics.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <map>
#include <algorithm>
#include <numeric>

StatisticsTests::StatisticsTests() : TestModule("StatisticsTests") {}

void StatisticsTests::registerTests() {
    addTest("ChecksumDistribution", [this]() { return testChecksumDistribution(); });
    addTest("LatencyDistribution", [this]() { return testLatencyDistribution(); });
    addTest("WorkerLoadBalance", [this]() { return testWorkerLoadBalance(); });
    addTest("ThroughputScaling", [this]() { return testThroughputScaling(); });
    addTest("ChecksumNonZero", [this]() { return testChecksumNonZero(); });
    addTest("ResultValueConsistency", [this]() { return testResultValueConsistency(); });
}

TestResult StatisticsTests::testChecksumDistribution() {
    const std::string name = "ChecksumDistribution";

    std::vector<uint64_t> checksums;
    for (int run = 0; run < 20; ++run) {
        std::vector<Event> events;
        for (int i = 0; i < 500; ++i) {
            events.emplace_back(uint64_t(run * 1000 + i), uint64_t(i + run * 500),
                                50.0 + run, uint32_t(10 + i % 20), 'B');
        }
        ReplayEngine engine(2);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();
        checksums.push_back(engine.getStateChecksum());
    }

    // all 20 runs with different inputs should produce different checksums
    std::sort(checksums.begin(), checksums.end());
    auto it = std::unique(checksums.begin(), checksums.end());
    size_t uniqueCount = std::distance(checksums.begin(), it);

    if (uniqueCount < 18) {
        return fail(name, "only " + std::to_string(uniqueCount) + "/20 unique checksums — poor distribution");
    }
    return pass(name, std::to_string(uniqueCount) + "/20 unique checksums");
}

TestResult StatisticsTests::testLatencyDistribution() {
    const std::string name = "LatencyDistribution";

    ReplayEngine engine(4);
    for (int i = 0; i < 10000; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    const auto& stats = engine.getStats();
    uint64_t minLat = stats.minLatencyNs.load();
    uint64_t maxLat = stats.maxLatencyNs.load();
    uint64_t totalLat = stats.totalLatencyNs.load();
    uint64_t count = stats.eventsProcessed.load();
    double avgLat = static_cast<double>(totalLat) / count;

    // min should be <= avg <= max
    if (static_cast<double>(minLat) > avgLat || avgLat > static_cast<double>(maxLat)) {
        return fail(name, "latency invariant violated: min <= avg <= max");
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    oss << "min=" << minLat << " avg=" << avgLat << " max=" << maxLat << " range=" << (maxLat - minLat);
    return pass(name, oss.str());
}

TestResult StatisticsTests::testWorkerLoadBalance() {
    const std::string name = "WorkerLoadBalance";
    const uint32_t numWorkers = 4;

    ReplayEngine engine(numWorkers);
    const int eventCount = 20000;
    for (int i = 0; i < eventCount; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    std::map<uint32_t, int> workerCounts;
    for (auto& r : results) {
        workerCounts[r.workerId]++;
    }

    // at least 2 workers should have processed events
    if (workerCounts.size() < 2) {
        return fail(name, "only " + std::to_string(workerCounts.size()) + " worker(s) did work");
    }

    std::ostringstream oss;
    oss << workerCounts.size() << " workers active: ";
    for (auto& [wid, cnt] : workerCounts) {
        oss << "w" << wid << "=" << cnt << " ";
    }
    return pass(name, oss.str());
}

TestResult StatisticsTests::testThroughputScaling() {
    const std::string name = "ThroughputScaling";
    const uint64_t eventCount = 5000;

    std::vector<Event> events;
    events.reserve(eventCount);
    for (uint64_t i = 0; i < eventCount; ++i) {
        events.emplace_back(i, i, 50.0, uint32_t(10), 'B');
    }

    std::map<uint32_t, double> throughputs;
    for (uint32_t threads : {1, 2, 4}) {
        ReplayEngine engine(threads);
        engine.submitEvents(events);
        auto start = std::chrono::steady_clock::now();
        engine.start();
        engine.waitForCompletion();
        auto end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();
        throughputs[threads] = eventCount / elapsed;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    for (auto& [t, tp] : throughputs) {
        oss << t << "t=" << tp << "e/s ";
    }
    return pass(name, oss.str());
}

TestResult StatisticsTests::testChecksumNonZero() {
    const std::string name = "ChecksumNonZero";

    for (int count : {1, 10, 100, 1000}) {
        std::vector<Event> events;
        for (int i = 0; i < count; ++i) {
            events.emplace_back(uint64_t(i), uint64_t(i + 1), 50.0 + i, uint32_t(10), 'B');
        }
        ReplayEngine engine(2);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();

        if (engine.getStateChecksum() == 0) {
            return fail(name, "zero checksum with " + std::to_string(count) + " events");
        }
    }
    return pass(name, "non-zero checksums for 1, 10, 100, 1000 events");
}

TestResult StatisticsTests::testResultValueConsistency() {
    const std::string name = "ResultValueConsistency";

    std::vector<Event> events;
    events.emplace_back(uint64_t(100), uint64_t(1), 50.0, uint32_t(10), 'B'); // NewOrder: 50*10=500
    events.emplace_back(uint64_t(200), uint64_t(2), 75.0, uint32_t(20), 'S'); // NewOrder: 75*20=1500

    ReplayEngine engine(2);
    engine.submitEvents(events);
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    if (results.size() != 2) {
        return fail(name, "wrong result count");
    }

    std::sort(results.begin(), results.end(),
        [](const ProcessedResult& a, const ProcessedResult& b) {
            return a.sequenceId < b.sequenceId;
        });

    if (std::abs(results[0].stateValue - 500.0) > 0.001) {
        return fail(name, "first result value wrong: " + std::to_string(results[0].stateValue));
    }
    if (std::abs(results[1].stateValue - 1500.0) > 0.001) {
        return fail(name, "second result value wrong: " + std::to_string(results[1].stateValue));
    }
    return pass(name, "result values match expected computations");
}
