/**
 * ASFMLogger Benchmark Infrastructure
 *
 * Shared utilities, test data generators, and measurement helpers
 * for all performance benchmark suites.
 */

#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>

// Project includes
#include "../src/structs/LogDataStructures.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Performance measurement utilities
class BenchmarkTimer {
public:
    void start() { start_time = std::chrono::high_resolution_clock::now(); }
    void stop() { end_time = std::chrono::high_resolution_clock::now(); }

    template<typename Duration = std::chrono::milliseconds>
    auto duration() const {
        return std::chrono::duration_cast<Duration>(end_time - start_time);
    }

    void reset() { start_time = end_time = std::chrono::high_resolution_clock::time_point{}; }

private:
    std::chrono::high_resolution_clock::time_point start_time, end_time;
};

// Memory tracking utilities
class MemoryTracker {
public:
    size_t getCurrentUsage() const {
        // Cross-platform memory usage
        return 0; // Stub for now
    }

    void reset() {
        // Reset tracking
    }
};

// Test data generators
class TestDataGenerator {
public:
    static LogMessageData GenerateLogMessage(LogMessageType type = LogMessageType::INFO,
                                           const std::string& component = "TestComponent",
                                           const std::string& message = "Test message content") {
        LogMessageData msg;
        // Required fields from LogMessageData structure
        msg.message_id = 99999;
        msg.instance_id = 1000;
        msg.type = type;
        msg.timestamp.seconds = 1234567890; // Fixed timestamp for consistency
        msg.timestamp.microseconds = 123456;
        msg.timestamp.milliseconds = 123;
        msg.process_id = 1234;
        msg.thread_id = 5678;
        msg.line_number = 100;

        // Copy strings (ensuring null termination with fixed-size arrays)
        strncpy(msg.message, message.c_str(), sizeof(msg.message) - 1);
        msg.message[sizeof(msg.message) - 1] = '\0';

        strncpy(msg.component, component.c_str(), sizeof(msg.component) - 1);
        msg.component[sizeof(msg.component) - 1] = '\0';

        strcpy(msg.function, "benchmarkFunction");
        strcpy(msg.file, "benchmark.cpp");
        strcpy(msg.severity_string, "INFO");

        // Reserve field (leave as zeros)

        return msg;
    }

    static std::vector<LogMessageData> GenerateBatch(size_t count) {
        std::vector<LogMessageData> batch;
        batch.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            batch.push_back(GenerateLogMessage(LogMessageType::INFO,
                                             "BatchComponent" + std::to_string(i),
                                             "Batch message " + std::to_string(i)));
        }

        return batch;
    }
};

// System resource monitoring
class SystemMonitor {
public:
    double getCpuUsage() const {
        // Platform-specific CPU monitoring
        return 0.0; // Stub
    }

    size_t getMemoryUsage() const {
        // Cross-platform memory monitoring
        return 0u; // Stub
    }

    void startMonitoring() {
        // Start resource tracking
    }

    void stopMonitoring() {
        // Stop resource tracking
    }
};

// Benchmark configuration constants
namespace BenchmarkConstants {
    const size_t WARMUP_ITERATIONS = 1000;
    const size_t MEASUREMENT_ITERATIONS = 5000;
    const size_t MAX_TEST_TIME_MS = 30000; // 30 seconds max per benchmark

    const size_t SMALL_BATCH_SIZE = 10;
    const size_t MEDIUM_BATCH_SIZE = 100;
    const size_t LARGE_BATCH_SIZE = 1000;

    const int NUM_THREADS_LOW = 2;
    const int NUM_THREADS_MEDIUM = 4;
    const int NUM_THREADS_HIGH = 8;
}

// Result reporting helpers
struct BenchmarkResult {
    std::string benchmark_name;
    size_t iterations;
    double duration_ms;
    double operations_per_second;
    size_t memory_usage_kb;
    double cpu_usage_percent;
};

class BenchmarkReporter {
public:
    static void report(const BenchmarkResult& result) {
        std::cout << "=== BENCHMARK RESULT: " << result.benchmark_name << " ===" << std::endl;
        std::cout << "Iterations: " << result.iterations << std::endl;
        std::cout << "Duration: " << result.duration_ms << "ms" << std::endl;
        std::cout << "Operations/sec: " << result.operations_per_second << std::endl;
        std::cout << "Memory usage: " << result.memory_usage_kb << "KB" << std::endl;
        std::cout << "CPU usage: " << result.cpu_usage_percent << "%" << std::endl;
        std::cout << std::endl;
    }

    static void reportSummary(const std::vector<BenchmarkResult>& results) {
        std::cout << "=== BENCHMARK SUMMARY ===" << std::endl;
        for (const auto& result : results) {
            std::cout << result.benchmark_name << ": "
                      << result.operations_per_second << " ops/sec" << std::endl;
        }
        std::cout << std::endl;
    }
};
