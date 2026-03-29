#ifndef TEST_METRICS_HPP
#define TEST_METRICS_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"

class MetricsTests : public TestModule {
public:
    MetricsTests();
    void registerTests() override;

private:
    TestResult testEventCountAccuracy();
    TestResult testLatencyTracking();
    TestResult testThroughputMeasurement();
    TestResult testStatsConsistency();
    TestResult testProducerConsumerParity();
    TestResult testLatencyBounds();
};

#endif
