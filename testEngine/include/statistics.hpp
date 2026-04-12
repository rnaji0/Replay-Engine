#ifndef TEST_STATISTICS_HPP
#define TEST_STATISTICS_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"
#include <vector>

class StatisticsTests : public TestModule {
public:
    StatisticsTests();
    void registerTests() override;

private:
    TestResult testChecksumDistribution();
    TestResult testLatencyDistribution();
    TestResult testWorkerLoadBalance();
    TestResult testThroughputScaling();
    TestResult testChecksumNonZero();
    TestResult testResultValueConsistency();
};

#endif
