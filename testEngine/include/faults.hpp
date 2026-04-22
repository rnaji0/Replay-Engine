#ifndef TEST_FAULTS_HPP
#define TEST_FAULTS_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"

class FaultTests : public TestModule {
public:
    FaultTests();
    void registerTests() override;

private:
    TestResult testGracefulShutdown();
    TestResult testEarlyStop();
    TestResult testDoubleStart();
    TestResult testDoubleStop();
    TestResult testZeroPriceEvents();
    TestResult testMaxValueEvents();
    TestResult testMassiveCancelStream();
    TestResult testEmptyQueueDrain();
    TestResult testDestructorSafety();
};

#endif
