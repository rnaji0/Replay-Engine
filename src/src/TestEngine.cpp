#include "TestEngine.hpp"
#include <iostream>
#include <iomanip>

TestEngine::TestEngine() {}
TestEngine::~TestEngine() {}

void TestEngine::registerModule(std::unique_ptr<TestModule> module) {
    module->registerTests();
    modules.push_back(std::move(module));
}

TestSuiteReport TestEngine::runAll() {
    TestSuiteReport report;

    std::cout << "\n";
    std::cout << "========================================================\n";
    std::cout << "  REPLAY ENGINE - COMPREHENSIVE TEST SUITE\n";
    std::cout << "========================================================\n\n";

    for (auto& mod : modules) {
        std::cout << "--- Module: " << mod->getName() << " ---\n";
        auto results = mod->runAll();
        for (auto& r : results) {
            report.addResult(r);

            const char* icon;
            switch (r.status) {
                case TestStatus::PASSED:  icon = "[PASS]"; break;
                case TestStatus::FAILED:  icon = "[FAIL]"; break;
                case TestStatus::ERROR:   icon = "[ERR ]"; break;
                case TestStatus::SKIPPED: icon = "[SKIP]"; break;
            }
            std::cout << "  " << icon << " " << r.testName;
            std::cout << " (" << std::fixed << std::setprecision(2) << r.durationMs << " ms)";
            if (!r.message.empty()) {
                std::cout << " - " << r.message;
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    return report;
}

TestSuiteReport TestEngine::runModule(const std::string& moduleName) {
    TestSuiteReport report;
    for (auto& mod : modules) {
        if (mod->getName() == moduleName) {
            auto results = mod->runAll();
            for (auto& r : results) {
                report.addResult(r);
            }
        }
    }
    return report;
}

void TestEngine::printReport(const TestSuiteReport& report) const {
    std::cout << "========================================================\n";
    std::cout << "  TEST SUITE SUMMARY\n";
    std::cout << "========================================================\n";
    std::cout << "  Total:   " << report.totalTests << "\n";
    std::cout << "  Passed:  " << report.passed << "\n";
    std::cout << "  Failed:  " << report.failed << "\n";
    std::cout << "  Errors:  " << report.errors << "\n";
    std::cout << "  Skipped: " << report.skipped << "\n";
    std::cout << "  Duration: " << std::fixed << std::setprecision(2)
              << report.totalDurationMs << " ms\n";
    std::cout << "  Pass Rate: " << std::fixed << std::setprecision(2)
              << report.passRate() << "%\n";
    std::cout << "========================================================\n";

    if (report.failed > 0 || report.errors > 0) {
        std::cout << "\n  FAILED/ERROR TESTS:\n";
        for (auto& r : report.allResults) {
            if (r.status == TestStatus::FAILED || r.status == TestStatus::ERROR) {
                std::cout << "    [" << r.statusString() << "] "
                          << r.moduleName << "::" << r.testName
                          << " - " << r.message << "\n";
            }
        }
    }

    std::cout << "\n  Result: " << (report.allPassed() ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n\n";
}
