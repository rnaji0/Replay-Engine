#ifndef TEST_REPLAY_HPP
#define TEST_REPLAY_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"
#include <vector>

class ReplayTests : public TestModule {
public:
    ReplayTests();
    void registerTests() override;

private:
    std::vector<Event> generateTestEvents(uint64_t count);

    TestResult testDeterministicReplay();
    TestResult testSequenceOrdering();
    TestResult testBitIdenticalChecksum();
    TestResult testMultipleRunConsistency();
    TestResult testSingleEventReplay();
    TestResult testEmptyReplay();
    TestResult testLargeScaleReplay();
    TestResult testMixedEventTypes();
    TestResult testDeterminismUnderVaryingThreads();
    TestResult testResultOrdering();
};

#endif
