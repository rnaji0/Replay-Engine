#ifndef TESTENGINE_HPP
#define TESTENGINE_HPP

#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include "TestModule.hpp"

struct TestSuiteReport {
    std::vector<TestResult> allResults;
    int totalTests = 0;
    int passed = 0;
    int failed = 0;
    int errors = 0;
    int skipped = 0;
    double totalDurationMs = 0.0;

    void addResult(const TestResult& r) {
        allResults.push_back(r);
        totalTests++;
        switch (r.status) {
            case TestStatus::PASSED:  passed++;  break;
            case TestStatus::FAILED:  failed++;  break;
            case TestStatus::ERROR:   errors++;  break;
            case TestStatus::SKIPPED: skipped++; break;
        }
        totalDurationMs += r.durationMs;
    }

    bool allPassed() const { return failed == 0 && errors == 0; }
    double passRate() const {
        int run = totalTests - skipped;
        return run > 0 ? (100.0 * passed / run) : 0.0;
    }
};

class TestEngine {
public:
    TestEngine();
    ~TestEngine();

    void registerModule(std::unique_ptr<TestModule> module);
    TestSuiteReport runAll();
    TestSuiteReport runModule(const std::string& moduleName);
    void printReport(const TestSuiteReport& report) const;

private:
    std::vector<std::unique_ptr<TestModule>> modules;
};

#endif
