/**
 * ASFMLogger Ultra-Specialized Core Component Testing
 * TASK 1.02A: SmartMessageQueue Deep-Dive Testing
 * Purpose: Exhaustive validation of intelligent queuing with priority preservation
 * Business Value: Performance foundation for enterprise logging - zero queue reliability risk (⭐⭐⭐⭐⭐)
 */

#ifndef TEST_CORE_SMART_MESSAGE_QUEUE_HPP
#define TEST_CORE_SMART_MESSAGE_QUEUE_HPP

// Test includes must come first for GTest
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>
#include <filesystem>
#include <fstream>

// Core logger includes
#include "../src/stateful/SmartMessageQueue.hpp"

// Queue testing components for isolated validation
namespace QueueTesting {

    /**
     * @brief Mock importance resolution for testing
     */
    enum class Importance {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERR = 4,
        CRITICAL = 5,
        MAX_IMPORTANCE
    };

    /**
     * @brief Test message data structure
     */
    struct TestMessage {
        std::string message;
        std::string level;
        std::string component;
        Importance importance;
        std::chrono::system_clock::time_point timestamp;
        size_t sequence_id;
        std::string correlation_id;

        TestMessage(const std::string& msg, const std::string& lvl, const std::string& comp,
                   Importance imp = Importance::INFO, size_t seq = 0)
            : message(msg), level(lvl), component(comp), importance(imp),
              timestamp(std::chrono::system_clock::now()), sequence_id(seq),
              correlation_id("correlation-" + std::to_string(seq)) {}
    };

} // namespace QueueTesting

#endif // TEST_CORE_SMART_MESSAGE_QUEUE_HPP
