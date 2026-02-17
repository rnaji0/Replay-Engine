#include "stress.hpp"
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>

StressTests::StressTests() : TestModule("StressTests") {}

void StressTests::registerTests() {
    addTest("HighThroughput", [this]() { return testHighThroughput(); });
    addTest("MaxThreads", [this]() { return testMaxThreads(); });
    addTest("RapidStartStop", [this]() { return testRapidStartStop(); });
    addTest("ConcurrentSubmission", [this]() { return testConcurrentSubmission(); });
    addTest("BurstTraffic", [this]() { return testBurstTraffic(); });
    addTest("SustainedLoad", [this]() { return testSustainedLoad(); });
    addTest("QueuePressure", [this]() { return testQueuePressure(); });
}

TestResult StressTests::testHighThroughput() {
    const std::string name = "HighThroughput";
    const uint64_t eventCount = 50000;

    std::vector<Event> events;
    events.reserve(eventCount);
    for (uint64_t i = 0; i < eventCount; ++i) {
        events.emplace_back(i * 10, 1000 + i, 100.0 + (i % 100), uint32_t(10 + i % 50), 'B');
    }

    ReplayEngine engine(4);
    engine.submitEvents(events);

    auto start = std::chrono::steady_clock::now();
    engine.start();
    engine.waitForCompletion();
    auto end = std::chrono::steady_clock::now();

    double elapsed = std::chrono::duration<double>(end - start).count();
    double eventsPerSec = eventCount / elapsed;

    auto results = engine.getResults();
    if (results.size() != eventCount) {
        return fail(name, "lost events: " + std::to_string(results.size()) + "/" + std::to_string(eventCount));
    }

    std::ostringstream oss;
    oss << std::fixed;
    oss.precision(0);
    oss << eventsPerSec << " events/sec (" << eventCount << " in "
        << std::setprecision(3) << elapsed << "s)";
    return pass(name, oss.str());
}

TestResult StressTests::testMaxThreads() {
    const std::string name = "MaxThreads";
    uint32_t maxThreads = std::thread::hardware_concurrency();
    if (maxThreads < 2) maxThreads = 2;

    std::vector<Event> events;
    for (uint64_t i = 0; i < 10000; ++i) {
        events.emplace_back(i, 100 + i, 50.0, uint32_t(10), 'B');
    }

    ReplayEngine engine(maxThreads);
    engine.submitEvents(events);
    engine.start();
    engine.waitForCompletion();

    auto results = engine.getResults();
    if (results.size() != 10000) {
        return fail(name, "lost events with " + std::to_string(maxThreads) + " threads");
    }

    return pass(name, "handled " + std::to_string(maxThreads) + " worker threads");
}

TestResult StressTests::testRapidStartStop() {
    const std::string name = "RapidStartStop";
    const int cycles = 20;

    for (int i = 0; i < cycles; ++i) {
        ReplayEngine engine(2);
        for (int j = 0; j < 100; ++j) {
            engine.submitEvent(Event(uint64_t(j), uint64_t(j), 10.0, uint32_t(1), 'B'));
        }
        engine.start();
        engine.waitForCompletion();

        if (engine.getResults().size() != 100) {
            return fail(name, "cycle " + std::to_string(i) + " lost events");
        }
    }
    return pass(name, std::to_string(cycles) + " rapid start/stop cycles completed");
}

TestResult StressTests::testConcurrentSubmission() {
    const std::string name = "ConcurrentSubmission";

    ReplayEngine engine(4);
    const int numProducers = 4;
    const int eventsPerProducer = 2000;

    std::vector<std::thread> producers;
    for (int p = 0; p < numProducers; ++p) {
        producers.emplace_back([&engine, p, eventsPerProducer]() {
            for (int i = 0; i < eventsPerProducer; ++i) {
                uint64_t oid = p * 100000 + i;
                engine.submitEvent(Event(uint64_t(i), oid, 50.0, uint32_t(10), 'B'));
            }
        });
    }

    for (auto& t : producers) t.join();

    engine.start();
    engine.waitForCompletion();

    uint64_t expected = numProducers * eventsPerProducer;
    auto results = engine.getResults();
    if (results.size() != expected) {
        return fail(name, "expected " + std::to_string(expected) + " got " + std::to_string(results.size()));
    }
    return pass(name, std::to_string(expected) + " events from " + std::to_string(numProducers) + " concurrent producers");
}

TestResult StressTests::testBurstTraffic() {
    const std::string name = "BurstTraffic";

    ReplayEngine engine(4);
    const int bursts = 10;
    const int eventsPerBurst = 1000;

    for (int b = 0; b < bursts; ++b) {
        for (int i = 0; i < eventsPerBurst; ++i) {
            engine.submitEvent(Event(uint64_t(b * eventsPerBurst + i), uint64_t(i), 100.0, uint32_t(10), 'S'));
        }
    }

    engine.start();
    engine.waitForCompletion();

    uint64_t expected = bursts * eventsPerBurst;
    if (engine.getResults().size() != expected) {
        return fail(name, "lost events during burst");
    }
    return pass(name, std::to_string(expected) + " events across " + std::to_string(bursts) + " bursts");
}

TestResult StressTests::testSustainedLoad() {
    const std::string name = "SustainedLoad";

    ReplayEngine engine(4);
    const uint64_t totalEvents = 20000;

    std::vector<Event> events;
    events.reserve(totalEvents);
    for (uint64_t i = 0; i < totalEvents; ++i) {
        events.emplace_back(i * 5, i, 75.0 + (i % 50), uint32_t(1 + i % 100), (i % 2) ? 'B' : 'S');
    }

    engine.submitEvents(events);

    auto start = std::chrono::steady_clock::now();
    engine.start();
    engine.waitForCompletion();
    auto end = std::chrono::steady_clock::now();

    auto results = engine.getResults();
    if (results.size() != totalEvents) {
        return fail(name, "lost events under sustained load");
    }

    double elapsed = std::chrono::duration<double>(end - start).count();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << totalEvents << " events in " << elapsed << "s";
    return pass(name, oss.str());
}

TestResult StressTests::testQueuePressure() {
    const std::string name = "QueuePressure";

    ReplayEngine engine(1);
    const uint64_t count = 10000;

    for (uint64_t i = 0; i < count; ++i) {
        engine.submitEvent(Event(i, i, 10.0, uint32_t(1), 'B'));
    }

    engine.start();
    engine.waitForCompletion();

    if (engine.getResults().size() != count) {
        return fail(name, "queue pressure caused event loss");
    }
    return pass(name, "single worker handled " + std::to_string(count) + " queued events");
}
