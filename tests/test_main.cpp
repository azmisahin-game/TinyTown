#include "test_framework.hpp"

#include <cstdlib>
#include <iostream>

namespace tt::test {

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

int& failureCount() {
    static int count = 0;
    return count;
}

const char*& currentTest() {
    static const char* name = "";
    return name;
}

Registrar::Registrar(const char* suite, const char* name, void (*fn)()) {
    registry().push_back(TestCase{suite, name, fn});
}

void reportFailure(const char* file, int line, const std::string& message) {
    ++failureCount();
    std::cout << "  FAIL [" << currentTest() << "] " << file << ":" << line << "\n"
              << "       " << message << "\n";
}

std::string describe(bool v) { return v ? "true" : "false"; }

int runAll() {
    int passed = 0;
    int failedTests = 0;

    const char* lastSuite = nullptr;
    for (const TestCase& test : registry()) {
        if (lastSuite == nullptr || std::string(lastSuite) != test.suite) {
            std::cout << "\n[" << test.suite << "]\n";
            lastSuite = test.suite;
        }

        currentTest() = test.name;
        const int before = failureCount();
        test.fn();

        if (failureCount() == before) {
            std::cout << "  ok   " << test.name << "\n";
            ++passed;
        } else {
            std::cout << "  FAILED " << test.name << "\n";
            ++failedTests;
        }
    }

    std::cout << "\n----------------------------------------\n";
    std::cout << passed << " passed, " << failedTests << " failed, "
              << failureCount() << " assertion failure(s)\n";
    return failedTests == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace tt::test

int main() { return tt::test::runAll(); }
