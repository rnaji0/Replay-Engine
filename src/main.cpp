#include <iostream>
#include <memory>
#include <chrono>
#include <map>

#include "TestEngine.hpp"
#include "ReplayEngine.hpp"
#include "replay.hpp"
#include "stress.hpp"
#include "memory.hpp"
#include "metrics.hpp"
#include "statistics.hpp"
#include "faults.hpp"
#include "visualization.hpp"

void runVisualizationDemo() {
    std::cout << "\n========================================================\n";
    std::cout << "  PERFORMANCE VISUALIZATION\n";
    std::cout << "========================================================\n";

    // throughput scaling chart
    const uint64_t eventCount = 5000;
    std::vector<Event> events;
    events.reserve(eventCount);
    for (uint64_t i = 0; i < eventCount; ++i) {
        events.emplace_back(i, i, 50.0 + (i % 100), uint32_t(10 + i % 50), (i % 2) ? 'B' : 'S');
    }

    std::map<uint32_t, double> throughputs;
    for (uint32_t threads : {1, 2, 4, 8}) {
        ReplayEngine engine(threads);
        engine.submitEvents(events);
        auto start = std::chrono::steady_clock::now();
        engine.start();
        engine.waitForCompletion();
        auto end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();
        throughputs[threads] = eventCount / elapsed;
    }
    Visualization::printThroughputChart(throughputs);

    // worker distribution with 4 threads
    {
        ReplayEngine engine(4);
        engine.submitEvents(events);
        engine.start();
        engine.waitForCompletion();
        Visualization::printWorkerDistribution(engine.getResults(), 4);
        Visualization::printLatencySummary(engine.getStats());
    }

    // determinism matrix
    std::cout << "\n  Determinism Verification\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    std::vector<uint64_t> checksums;
    std::vector<uint32_t> threadCounts = {1, 2, 4};
    for (uint32_t t : threadCounts) {
        for (int run = 0; run < 3; ++run) {
            ReplayEngine engine(t);
            engine.submitEvents(events);
            engine.start();
            engine.waitForCompletion();
            checksums.push_back(engine.getStateChecksum());
        }
    }
    Visualization::printDeterminismMatrix(checksums, threadCounts);
}

int main() {
    TestEngine testEngine;

    testEngine.registerModule(std::make_unique<ReplayTests>());
    testEngine.registerModule(std::make_unique<StressTests>());
    testEngine.registerModule(std::make_unique<MemoryTests>());
    testEngine.registerModule(std::make_unique<MetricsTests>());
    testEngine.registerModule(std::make_unique<StatisticsTests>());
    testEngine.registerModule(std::make_unique<FaultTests>());

    auto report = testEngine.runAll();
    testEngine.printReport(report);

    runVisualizationDemo();

    return report.allPassed() ? 0 : 1;
}
