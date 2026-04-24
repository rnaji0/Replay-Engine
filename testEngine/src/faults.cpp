#include "faults.hpp"
#include <limits>
#include <sstream>
#include <thread>
#include <chrono>

FaultTests::FaultTests() : TestModule("FaultTests") {}

void FaultTests::registerTests() {
    addTest("GracefulShutdown", [this]() { return testGracefulShutdown(); });
    addTest("EarlyStop", [this]() { return testEarlyStop(); });
    addTest("DoubleStart", [this]() { return testDoubleStart(); });
    addTest("DoubleStop", [this]() { return testDoubleStop(); });
    addTest("ZeroPriceEvents", [this]() { return testZeroPriceEvents(); });
    addTest("MaxValueEvents", [this]() { return testMaxValueEvents(); });
    addTest("MassiveCancelStream", [this]() { return testMassiveCancelStream(); });
    addTest("EmptyQueueDrain", [this]() { return testEmptyQueueDrain(); });
    addTest("DestructorSafety", [this]() { return testDestructorSafety(); });
}

TestResult FaultTests::testGracefulShutdown() {
    const std::string name = "GracefulShutdown";

    ReplayEngine engine(4);
    for (int i = 0; i < 10000; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    if (!engine.getResults().size()) {
        return fail(name, "no results after graceful shutdown");
    }
    return pass(name, "engine shut down gracefully with " + std::to_string(engine.getResults().size()) + " results");
}

TestResult FaultTests::testEarlyStop() {
    const std::string name = "EarlyStop";

    ReplayEngine engine(2);
    for (int i = 0; i < 10000; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    engine.stop();

    // should not crash, results count may be less than submitted
    return pass(name, "early stop handled, " + std::to_string(engine.getResults().size()) + " events processed");
}

TestResult FaultTests::testDoubleStart() {
    const std::string name = "DoubleStart";

    ReplayEngine engine(2);
    for (int i = 0; i < 100; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 50.0, uint32_t(10), 'B'));
    }
    engine.start();
    engine.start(); // second start should be a no-op
    engine.waitForCompletion();

    if (engine.getResults().size() != 100) {
        return fail(name, "double start corrupted results");
    }
    return pass(name, "double start handled safely");
}

TestResult FaultTests::testDoubleStop() {
    const std::string name = "DoubleStop";

    ReplayEngine engine(2);
    engine.submitEvent(Event(uint64_t(1), uint64_t(1), 50.0, uint32_t(10), 'B'));
    engine.start();
    engine.waitForCompletion();
    engine.stop(); // second stop should be safe

    return pass(name, "double stop handled safely");
}

TestResult FaultTests::testZeroPriceEvents() {
    const std::string name = "ZeroPriceEvents";

    ReplayEngine engine(2);
    for (int i = 0; i < 100; ++i) {
        engine.submitEvent(Event(uint64_t(i), uint64_t(i), 0.0, uint32_t(0), 'B'));
    }
    engine.start();
    engine.waitForCompletion();

    if (engine.getResults().size() != 100) {
        return fail(name, "zero-price events dropped");
    }
    return pass(name, "100 zero-price events processed");
}

TestResult FaultTests::testMaxValueEvents() {
    const std::string name = "MaxValueEvents";

    ReplayEngine engine(2);
    engine.submitEvent(Event(
        UINT64_MAX - 1,
        UINT64_MAX - 1,
        std::numeric_limits<double>::max(),
        UINT32_MAX,
        'B'
    ));
    engine.submitEvent(Event(
        uint64_t(0), uint64_t(0), 0.0, uint32_t(0), 'S'
    ));
    engine.start();
    engine.waitForCompletion();

    if (engine.getResults().size() != 2) {
        return fail(name, "extreme value events dropped");
    }
    return pass(name, "extreme boundary values handled");
}

TestResult FaultTests::testMassiveCancelStream() {
    const std::string name = "MassiveCancelStream";

    ReplayEngine engine(2);
    // submit one new order then flood with cancels
    engine.submitEvent(Event(uint64_t(1), uint64_t(1), 100.0, uint32_t(50), 'B'));
    for (uint64_t i = 2; i <= 1000; ++i) {
        engine.submitEvent(Event(i)); // CancelOrder
    }
    engine.start();
    engine.waitForCompletion();

    if (engine.getResults().size() != 1000) {
        return fail(name, "cancel stream lost events");
    }
    return pass(name, "999 cancels after 1 order processed correctly");
}

TestResult FaultTests::testEmptyQueueDrain() {
    const std::string name = "EmptyQueueDrain";

    LFQueue<Event> q;
    Event e;
    bool result = q.dequeue(e);
    if (result) {
        return fail(name, "dequeue from empty queue returned true");
    }
    if (!q.isEmpty()) {
        return fail(name, "empty queue reports non-empty");
    }
    return pass(name, "empty queue behaves correctly");
}

TestResult FaultTests::testDestructorSafety() {
    const std::string name = "DestructorSafety";

    for (int i = 0; i < 30; ++i) {
        auto engine = new ReplayEngine(2);
        for (int j = 0; j < 500; ++j) {
            engine->submitEvent(Event(uint64_t(j), uint64_t(j), 50.0, uint32_t(10), 'B'));
        }
        engine->start();
        engine->waitForCompletion();
        delete engine;
    }
    return pass(name, "30 heap-allocated engine cycles destroyed safely");
}
