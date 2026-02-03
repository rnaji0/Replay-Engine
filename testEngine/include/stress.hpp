#ifndef TEST_STRESS_HPP
#define TEST_STRESS_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"

class StressTests : public TestModule {
public:
    StressTests();
    void registerTests() override;

private:
    TestResult testHighThroughput();
    TestResult testMaxThreads();
    TestResult testRapidStartStop();
    TestResult testConcurrentSubmission();
    TestResult testBurstTraffic();
    TestResult testSustainedLoad();
    TestResult testQueuePressure();
};

#endif
