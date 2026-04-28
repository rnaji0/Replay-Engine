#include "visualization.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <numeric>

void Visualization::printBar(const std::string& label, double value, double maxValue, int barWidth) {
    int filled = (maxValue > 0) ? static_cast<int>((value / maxValue) * barWidth) : 0;
    filled = std::min(filled, barWidth);

    std::cout << std::setw(20) << std::left << label << " |";
    for (int i = 0; i < barWidth; ++i) {
        std::cout << (i < filled ? '#' : ' ');
    }
    std::cout << "| " << std::fixed << std::setprecision(0) << value << "\n";
}

void Visualization::printHistogram(const std::string& title, const std::vector<double>& values, int bins) {
    if (values.empty()) return;

    double minVal = *std::min_element(values.begin(), values.end());
    double maxVal = *std::max_element(values.begin(), values.end());
    double range = maxVal - minVal;
    if (range == 0) range = 1;
    double binWidth = range / bins;

    std::vector<int> counts(bins, 0);
    for (double v : values) {
        int bin = static_cast<int>((v - minVal) / binWidth);
        if (bin >= bins) bin = bins - 1;
        counts[bin]++;
    }

    int maxCount = *std::max_element(counts.begin(), counts.end());

    std::cout << "\n  " << title << "\n";
    std::cout << "  " << std::string(60, '-') << "\n";

    for (int i = 0; i < bins; ++i) {
        double lo = minVal + i * binWidth;
        double hi = lo + binWidth;
        std::ostringstream label;
        label << std::fixed << std::setprecision(0) << lo << "-" << hi;
        printBar(label.str(), counts[i], maxCount, 30);
    }
}

void Visualization::printWorkerDistribution(const std::vector<ProcessedResult>& results, uint32_t numWorkers) {
    std::map<uint32_t, int> workerCounts;
    for (auto& r : results) {
        workerCounts[r.workerId]++;
    }

    int maxCount = 0;
    for (auto& [wid, cnt] : workerCounts) {
        maxCount = std::max(maxCount, cnt);
    }

    std::cout << "\n  Worker Load Distribution\n";
    std::cout << "  " << std::string(60, '-') << "\n";

    for (uint32_t w = 0; w < numWorkers; ++w) {
        int count = workerCounts.count(w) ? workerCounts[w] : 0;
        std::string label = "Worker " + std::to_string(w);
        printBar(label, count, maxCount, 40);
    }
}

void Visualization::printThroughputChart(const std::map<uint32_t, double>& threadThroughputs) {
    double maxTP = 0;
    for (auto& [t, tp] : threadThroughputs) {
        maxTP = std::max(maxTP, tp);
    }

    std::cout << "\n  Throughput vs Thread Count\n";
    std::cout << "  " << std::string(60, '-') << "\n";

    for (auto& [threads, tp] : threadThroughputs) {
        std::string label = std::to_string(threads) + " thread(s)";
        printBar(label, tp, maxTP, 40);
    }
}

void Visualization::printLatencySummary(const EngineStats& stats) {
    uint64_t processed = stats.eventsProcessed.load();
    if (processed == 0) return;

    uint64_t minLat = stats.minLatencyNs.load();
    uint64_t maxLat = stats.maxLatencyNs.load();
    uint64_t totalLat = stats.totalLatencyNs.load();
    double avgLat = static_cast<double>(totalLat) / processed;

    std::cout << "\n  Latency Summary (nanoseconds)\n";
    std::cout << "  " << std::string(40, '-') << "\n";
    std::cout << "  Min:     " << std::setw(12) << minLat << " ns\n";
    std::cout << "  Avg:     " << std::setw(12) << std::fixed << std::setprecision(0) << avgLat << " ns\n";
    std::cout << "  Max:     " << std::setw(12) << maxLat << " ns\n";
    std::cout << "  Total:   " << std::setw(12) << totalLat << " ns\n";
    std::cout << "  Events:  " << std::setw(12) << processed << "\n";
}

void Visualization::printDeterminismMatrix(const std::vector<uint64_t>& checksums, const std::vector<uint32_t>& threadCounts) {
    std::cout << "\n  Determinism Verification Matrix\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    std::cout << "  " << std::setw(10) << "Threads";

    for (size_t j = 0; j < checksums.size(); ++j) {
        std::cout << std::setw(18) << ("Run " + std::to_string(j + 1));
    }
    std::cout << "  Match\n";
    std::cout << "  " << std::string(60, '-') << "\n";

    size_t idx = 0;
    for (size_t t = 0; t < threadCounts.size(); ++t) {
        std::cout << "  " << std::setw(10) << threadCounts[t];
        uint64_t first = checksums[idx];
        bool allMatch = true;
        size_t runsPerThread = checksums.size() / threadCounts.size();
        for (size_t r = 0; r < runsPerThread && idx < checksums.size(); ++r, ++idx) {
            std::cout << std::setw(18) << checksums[idx];
            if (checksums[idx] != first) allMatch = false;
        }
        std::cout << (allMatch ? "  YES" : "  NO") << "\n";
    }
}
