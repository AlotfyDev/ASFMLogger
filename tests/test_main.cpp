/**
 * ASFMLogger Test Suite Main
 *
 * Comprehensive test suite for all ASFMLogger components
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

// Test configuration
class ASFMLoggerTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Initialize test environment
        std::cout << "Setting up ASFMLogger test environment..." << std::endl;
    }

    void TearDown() override {
        // Cleanup test environment
        std::cout << "Tearing down ASFMLogger test environment..." << std::endl;
    }
};

int main(int argc, char **argv) {
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    // Add custom test environment
    ::testing::AddGlobalTestEnvironment(new ASFMLoggerTestEnvironment());

    // Set test output format
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";

    // Run all tests
    return RUN_ALL_TESTS();
}

// Global test fixtures
class ASFMLoggerTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
    }

    void TearDown() override {
        // Common cleanup for all tests
    }
};

// Performance test fixture
class PerformanceTestFixture : public ASFMLoggerTestBase {
protected:
    static const int PERFORMANCE_TEST_ITERATIONS = 10000;
    static const int STRESS_TEST_ITERATIONS = 100000;

    std::chrono::high_resolution_clock::time_point start_time_;

    void StartTimer() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    double GetElapsedMilliseconds() {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end_time - start_time_).count();
    }

    void ExpectPerformanceUnder(double max_ms, const std::string& operation) {
        double elapsed = GetElapsedMilliseconds();
        EXPECT_LT(elapsed, max_ms)
            << operation << " took " << elapsed << "ms, expected under " << max_ms << "ms";
    }
};

// Thread safety test fixture
class ThreadSafetyTestFixture : public ASFMLoggerTestBase {
protected:
    static const int NUM_THREADS = 10;
    static const int OPERATIONS_PER_THREAD = 1000;

    std::atomic<int> counter_{0};
    std::atomic<bool> error_flag_{false};
    mutable std::mutex error_mutex_;
    std::vector<std::string> errors_;

    void RecordError(const std::string& error) {
        std::lock_guard<std::mutex> lock(error_mutex_);
        errors_.push_back(error);
        error_flag_ = true;
    }

    bool HadErrors() const {
        return error_flag_;
    }

    std::vector<std::string> GetErrors() const {
        std::lock_guard<std::mutex> lock(error_mutex_);
        return errors_;
    }
};

// Test macros for common patterns
#define EXPECT_DURATION_UNDER(max_ms, operation) \
    do { \
        auto start = std::chrono::high_resolution_clock::now(); \
        operation; \
        auto end = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration<double, std::milli>(end - start).count(); \
        EXPECT_LT(duration, max_ms) << "Operation took " << duration << "ms, expected under " << max_ms << "ms"; \
    } while(0)

#define EXPECT_NO_THROW_MSG(expression, message) \
    try { \
        expression; \
    } catch (const std::exception& e) { \
        FAIL() << message << ": " << e.what(); \
    } catch (...) { \
        FAIL() << message << ": Unknown exception"; \
    }

// Memory leak detection helper
#ifdef _MSC_VER
#include <crtdbg.h>
#define ENABLE_MEMORY_LEAK_DETECTION() \
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
#define ENABLE_MEMORY_LEAK_DETECTION()
#endif