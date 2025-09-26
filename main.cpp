#include <iostream>
#include <thread>


long long counter = 0;
int testsPassed = 0;
int testsFailed = 0;
int threadCycle = 10;
int testsRan = 1000000;


void worker() {
    for (int i = 0; i < threadCycle; i++) {
        counter++;
        std::this_thread::yield(); 
    }
}

int runTest() {
    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    return counter;
}

void testController() {
     for (int i = 0; i < testsRan; i++) {
        int testResult = runTest();
        if (testResult != threadCycle * 6) {
            std::cout << "Test " << i << " FAILED: counter = " << testResult << std::endl;
            testsFailed++;
        } else {
            std::cout << "Test " << i << " PASSED: counter = " << testResult << std::endl;
            testsPassed++;
        }
    counter = 0;
    }

    std::cout << std::endl;
    std::cout << "Tests Passed: " << testsPassed << std::endl;
    std::cout << "Tests Failed: " << testsFailed << std::endl;
    std::cout << "Pass Rate: " << (testsPassed / (float)(testsPassed + testsFailed)) * 100 << "%" << std::endl;
}


int main() {
    testController();
    return 0;
}
