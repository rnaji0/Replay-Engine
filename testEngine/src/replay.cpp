#include "replay.hpp"
#include <sstream>
#include <algorithm>

ReplayTests::ReplayTests() : TestModule("ReplayDeterminism") {}

void ReplayTests::registerTests() {
    addTest("DeterministicReplay", [this]() { return testDeterministicReplay(); });
    addTest("SequenceOrdering", [this]() { return testSequenceOrdering(); });
    addTest("BitIdenticalChecksum", [this]() { return testBitIdenticalChecksum(); });
    addTest("MultipleRunConsistency", [this]() { return testMultipleRunConsistency(); });
    addTest("SingleEventReplay", [this]() { return testSingleEventReplay(); });
    addTest("EmptyReplay", [this]() { return testEmptyReplay(); });
    addTest("LargeScaleReplay", [this]() { return testLargeScaleReplay(); });
    addTest("MixedEventTypes", [this]() { return testMixedEventTypes(); });
    addTest("DeterminismVaryingThreads", [this]() { return testDeterminismUnderVaryingThreads(); });
    addTest("ResultOrdering", [this]() { return testResultOrdering(); });
}

std::vector<Event> ReplayTests::generateTestEvents(uint64_t count) {
    std::vector<Event> events;
    events.reserve(count);
    for (uint64_t i = 0; i < count; ++i) {
        uint64_t ts = 1000 + i * 10;
        uint64_t oid = 100 + i;
        double price = 50.0 + (i % 100) * 0.5;
        uint32_t qty = 10 + (i % 50);
        char action = (i % 2 == 0) ? 'B' : 'S';
        events.emplace_back(ts, oid, price, qty, action);
    }
    return events;
}

TestResult ReplayTests::testDeterministicReplay() {
    const std::string name = "DeterministicReplay";
    auto events = generateTestEvents(1000);

    ReplayEngine engine1(2);
    engine1.submitEvents(events);
    engine1.start();
    engine1.waitForCompletion();
    uint64_t checksum1 = engine1.getStateChecksum();

    ReplayEngine engine2(2);
    engine2.submitEvents(events);
    engine2.start();
    engine2.waitForCompletion();
    uint64_t checksum2 = engine2.getStateChecksum();

    if (checksum1 == checksum2) {
        return pass(name, "checksums match across 2 runs");
    }
    std::ostringstream oss;
    oss << "checksum mismatch: " << checksum1 << " vs " << checksum2;
    return fail(name, oss.str());
}

TestResult ReplayTests::testSequenceOrdering() {
    const std::string name = "SequenceOrdering";
    auto events = generateTestEvents(500);

    ReplayEngine engine(4);
    engine.submitEvents(events);
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    if (results.size() != 500) {
        return fail(name, "wrong result count: " + std::to_string(results.size()));
    }

    std::sort(results.begin(), results.end(),
        [](const ProcessedResult& a, const ProcessedResult& b) {
            return a.sequenceId < b.sequenceId;
        });

    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i].sequenceId != i + 1) {
            return fail(name, "missing sequence ID " + std::to_string(i + 1));
        }
    }
    return pass(name, "all 500 sequence IDs present and contiguous");
}

TestResult ReplayTests::testBitIdenticalChecksum() {
    const std::string name = "BitIdenticalChecksum";
    auto events = generateTestEvents(2000);

    const int runs = 5;
    uint64_t checksums[5];

    for (int i = 0; i < runs; ++i) {
        ReplayEngine engine(3);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();
        checksums[i] = engine.getStateChecksum();
    }

    for (int i = 1; i < runs; ++i) {
        if (checksums[i] != checksums[0]) {
            std::ostringstream oss;
            oss << "run " << i << " checksum " << checksums[i]
                << " differs from run 0 checksum " << checksums[0];
            return fail(name, oss.str());
        }
    }
    return pass(name, "5 runs produced identical checksums");
}

TestResult ReplayTests::testMultipleRunConsistency() {
    const std::string name = "MultipleRunConsistency";
    auto events = generateTestEvents(300);

    const int runs = 10;
    std::vector<uint64_t> checksums;
    std::vector<size_t> resultCounts;

    for (int i = 0; i < runs; ++i) {
        ReplayEngine engine(2);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();
        checksums.push_back(engine.getStateChecksum());
        resultCounts.push_back(engine.getResults().size());
    }

    for (int i = 1; i < runs; ++i) {
        if (checksums[i] != checksums[0]) {
            return fail(name, "checksum diverged on run " + std::to_string(i));
        }
        if (resultCounts[i] != resultCounts[0]) {
            return fail(name, "result count diverged on run " + std::to_string(i));
        }
    }
    return pass(name, "10 runs consistent");
}

TestResult ReplayTests::testSingleEventReplay() {
    const std::string name = "SingleEventReplay";

    Event e(1000, 1, 99.5, 100, 'B');
    ReplayEngine engine(1);
    engine.submitEvent(e);
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    if (results.size() != 1) {
        return fail(name, "expected 1 result, got " + std::to_string(results.size()));
    }
    if (results[0].sequenceId != 1) {
        return fail(name, "wrong sequence ID");
    }
    return pass(name);
}

TestResult ReplayTests::testEmptyReplay() {
    const std::string name = "EmptyReplay";

    ReplayEngine engine(2);
    engine.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    engine.stop();

    auto results = engine.getResults();
    if (!results.empty()) {
        return fail(name, "expected 0 results from empty engine");
    }
    if (engine.getStateChecksum() != 0) {
        return fail(name, "expected zero checksum for empty run");
    }
    return pass(name, "empty engine produces no results and zero checksum");
}

TestResult ReplayTests::testLargeScaleReplay() {
    const std::string name = "LargeScaleReplay";
    auto events = generateTestEvents(10000);

    ReplayEngine engine(4);
    engine.submitEvents(events);
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    if (results.size() != 10000) {
        return fail(name, "expected 10000 results, got " + std::to_string(results.size()));
    }

    uint64_t checksum = engine.getStateChecksum();
    if (checksum == 0) {
        return fail(name, "checksum is zero for 10k events — suspicious");
    }

    std::ostringstream oss;
    oss << "10k events processed, checksum=" << checksum;
    return pass(name, oss.str());
}

TestResult ReplayTests::testMixedEventTypes() {
    const std::string name = "MixedEventTypes";

    std::vector<Event> events;
    events.emplace_back(1000, 1, 100.0, 50, 'B');          // NewOrder
    events.emplace_back(uint64_t(1));                        // CancelOrder
    events.emplace_back(1020, 2, 101.0, 30, 'S', true);    // Trade
    events.emplace_back(uint64_t(2), 105.0, uint32_t(25));  // ModifyOrder
    events.emplace_back(uint64_t(1040), 99.5);              // QuoteUpdate

    uint64_t checksums[3];
    for (int i = 0; i < 3; ++i) {
        ReplayEngine engine(2);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();
        checksums[i] = engine.getStateChecksum();
    }

    if (checksums[0] != checksums[1] || checksums[1] != checksums[2]) {
        return fail(name, "mixed event types produced non-deterministic checksums");
    }
    return pass(name, "all 5 event types processed deterministically");
}

TestResult ReplayTests::testDeterminismUnderVaryingThreads() {
    const std::string name = "DeterminismVaryingThreads";
    auto events = generateTestEvents(1000);

    uint64_t referenceChecksum;
    {
        ReplayEngine engine(1);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();
        referenceChecksum = engine.getStateChecksum();
    }

    for (uint32_t threads : {2, 4, 8}) {
        ReplayEngine engine(threads);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();

        if (engine.getStateChecksum() != referenceChecksum) {
            return fail(name, "checksum differs with " + std::to_string(threads) + " threads");
        }
    }
    return pass(name, "identical checksums with 1, 2, 4, and 8 threads");
}

TestResult ReplayTests::testResultOrdering() {
    const std::string name = "ResultOrdering";
    auto events = generateTestEvents(200);

    ReplayEngine engine(4);
    engine.submitEvents(events);
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    if (results.size() != 200) {
        return fail(name, "wrong result count");
    }

    std::sort(results.begin(), results.end(),
        [](const ProcessedResult& a, const ProcessedResult& b) {
            return a.sequenceId < b.sequenceId;
        });

    for (size_t i = 1; i < results.size(); ++i) {
        if (results[i].sequenceId <= results[i - 1].sequenceId) {
            return fail(name, "duplicate or misordered sequence IDs");
        }
    }
    return pass(name, "all results have unique, ordered sequence IDs");
}
