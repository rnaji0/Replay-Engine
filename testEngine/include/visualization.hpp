#ifndef TEST_VISUALIZATION_HPP
#define TEST_VISUALIZATION_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"
#include <string>
#include <vector>
#include <map>

class Visualization {
public:
    static void printBar(const std::string& label, double value, double maxValue, int barWidth = 40);
    static void printHistogram(const std::string& title, const std::vector<double>& values, int bins = 10);
    static void printWorkerDistribution(const std::vector<ProcessedResult>& results, uint32_t numWorkers);
    static void printThroughputChart(const std::map<uint32_t, double>& threadThroughputs);
    static void printLatencySummary(const EngineStats& stats);
    static void printDeterminismMatrix(const std::vector<uint64_t>& checksums, const std::vector<uint32_t>& threadCounts);
};

#endif
