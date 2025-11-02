#ifndef TESTMODULE_HPP
#define TESTMODULE_HPP

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <sstream>

enum class TestStatus { PASSED, FAILED, SKIPPED, ERROR };

struct TestResult {
    std::string testName;
    std::string moduleName;
    TestStatus status;
    std::string message;
    double durationMs;

    std::string statusString() const {
        switch (status) {
            case TestStatus::PASSED:  return "PASSED";
            case TestStatus::FAILED:  return "FAILED";
            case TestStatus::SKIPPED: return "SKIPPED";
            case TestStatus::ERROR:   return "ERROR";
        }
        return "UNKNOWN";
    }
};

struct TestCase {
    std::string name;
    std::function<TestResult()> run;
};

class TestModule {
public:
    explicit TestModule(const std::string& name) : moduleName(name) {}
    virtual ~TestModule() = default;

    virtual void setup() {}
    virtual void teardown() {}
    virtual void registerTests() = 0;

    const std::string& getName() const { return moduleName; }
    const std::vector<TestCase>& getTests() const { return tests; }

    std::vector<TestResult> runAll() {
        std::vector<TestResult> results;
        setup();
        for (auto& tc : tests) {
            auto start = std::chrono::steady_clock::now();
            try {
                TestResult r = tc.run();
                auto end = std::chrono::steady_clock::now();
                r.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
                r.moduleName = moduleName;
                results.push_back(r);
            } catch (const std::exception& ex) {
                auto end = std::chrono::steady_clock::now();
                TestResult r;
                r.testName = tc.name;
                r.moduleName = moduleName;
                r.status = TestStatus::ERROR;
                r.message = std::string("Exception: ") + ex.what();
                r.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
                results.push_back(r);
            }
        }
        teardown();
        return results;
    }

protected:
    void addTest(const std::string& name, std::function<TestResult()> fn) {
        tests.push_back({name, fn});
    }

    TestResult pass(const std::string& name, const std::string& msg = "") {
        return {name, moduleName, TestStatus::PASSED, msg, 0.0};
    }

    TestResult fail(const std::string& name, const std::string& msg) {
        return {name, moduleName, TestStatus::FAILED, msg, 0.0};
    }

    std::string moduleName;
    std::vector<TestCase> tests;
};

#endif
