#ifndef TEST_MEMORY_HPP
#define TEST_MEMORY_HPP

#include "TestModule.hpp"
#include "ReplayEngine.hpp"

class MemoryTests : public TestModule {
public:
    MemoryTests();
    void registerTests() override;

private:
    TestResult testCacheAlignment();
    TestResult testNoLeakOnDestruction();
    TestResult testRepeatedAllocDealloc();
    TestResult testLargeEventBatch();
    TestResult testQueueCleanup();
    TestResult testEventSizeCompact();
    TestResult testAlignedAtomics();
};

#endif
