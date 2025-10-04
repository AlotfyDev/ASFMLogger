/**
 * ASFMLogger Ultra-Specialized Core Component Testing
 * TASK 1.02A: SmartMessageQueue Deep-Dive Testing
 * Purpose: Exhaustive validation of intelligent queuing with priority preservation
 * Business Value: Performance foundation for enterprise logging - zero queue reliability risk (⭐⭐⭐⭐⭐)
 */

// Include the header first
#include "test_core_smart_message_queue.hpp"

// Queue testing components for isolated validation
namespace QueueTesting {

    /**
     * @brief Enhanced SmartMessageQueue with additional testing hooks
     */
    class TestableSmartMessageQueue {
    public:
        explicit TestableSmartMessageQueue(size_t capacity = 1000)
            : capacity_(capacity), current_size_(0) {
            // Initialize priority queues (higher importance = lower index for priority_queue)
            priority_queues_.resize(static_cast<size_t>(Importance::MAX_IMPORTANCE));
        }

        bool Enqueue(const TestMessage& message) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            if (current_size_ >= capacity_) {
                if (!PerformIntelligentEviction(1)) { // Try to evict one message
                    return false; // Queue is full and eviction failed
                }
            }

            size_t priority_idx = static_cast<size_t>(message.importance);
            if (priority_idx >= priority_queues_.size()) {
                priority_idx = static_cast<size_t>(Importance::INFO); // Default to INFO
            }

            priority_queues_[priority_idx].push(message);
            current_size_++;

            return true;
        }

        bool EnqueueWithEviction(const TestMessage& message) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            if (current_size_ >= capacity_) {
                if (!PerformIntelligentEviction(1)) {
                    return false;
                }
            }

            return EnqueueUnlocked(message, lock);
        }

        bool Dequeue(TestMessage& message) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Find the highest priority message
            for (size_t priority = 0; priority < priority_queues_.size(); ++priority) {
                if (!priority_queues_[priority].empty()) {
                    message = priority_queues_[priority].front();
                    priority_queues_[priority].pop();
                    current_size_--;
                    return true;
                }
            }

            return false; // Queue is empty
        }

        size_t GetSize() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return current_size_;
        }

        size_t GetCapacity() const { return capacity_; }

        bool IsEmpty() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return current_size_ == 0;
        }

        bool IsFull() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return current_size_ >= capacity_;
        }

        void Clear() {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            for (auto& queue : priority_queues_) {
                while (!queue.empty()) {
                    queue.pop();
                }
            }
            current_size_ = 0;
        }

        // Enhanced testing utilities
        std::vector<size_t> GetQueueSizes() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            std::vector<size_t> sizes;
            for (const auto& queue : priority_queues_) {
                sizes.push_back(queue.size());
            }
            return sizes;
        }

        std::vector<TestMessage> GetQueueContents(size_t priority_level) const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            std::vector<TestMessage> contents;

            if (priority_level < priority_queues_.size()) {
                auto queue_copy = priority_queues_[priority_level]; // Copy for thread safety
                while (!queue_copy.empty()) {
                    contents.push_back(queue_copy.front());
                    queue_copy.pop();
                }
            }

            return contents;
        }

        Importance GetHighestPriorityAvailable() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            for (size_t priority = 0; priority < priority_queues_.size(); ++priority) {
                if (!priority_queues_[priority].empty()) {
                    return static_cast<Importance>(priority);
                }
            }
            return Importance::MAX_IMPORTANCE; // Invalid, queue is empty
        }

    private:
        size_t capacity_;
        std::atomic<size_t> current_size_;
        mutable std::mutex queue_mutex_; // Use std::mutex for universal compatibility

        // Priority queues: index 0 = highest priority (CRITICAL), index 5 = lowest priority (TRACE)
        std::vector<std::queue<TestMessage>> priority_queues_;

        bool EnqueueUnlocked(const TestMessage& message, std::unique_lock<std::mutex>& lock) {
            size_t priority_idx = static_cast<size_t>(message.importance);
            if (priority_idx >= priority_queues_.size()) {
                priority_idx = static_cast<size_t>(Importance::INFO);
            }

            priority_queues_[priority_idx].push(message);
            current_size_++;

            return true;
        }

        bool PerformIntelligentEviction(size_t target_eviction_count) {
            // Intelligent eviction: remove lowest priority messages first
            // Preserve 80% of higher priority messages, aggressively evict lower priorities

            size_t evicted = 0;

            // First, try to evict from lowest priorities (TRACE, DEBUG)
            for (int priority = static_cast<int>(Importance::MAX_IMPORTANCE) - 1; priority >= 0 && evicted < target_eviction_count; --priority) {
                if (evicted >= target_eviction_count) break;

                auto priority_level = static_cast<Importance>(priority);
                size_t eviction_target = CalculateEvictionTarget(priority_level);

                while (!priority_queues_[priority].empty() && eviction_target > 0 && evicted < target_eviction_count) {
                    priority_queues_[priority].pop();
                    current_size_--;
                    evicted++;
                    eviction_target--;
                }
            }

            return evicted > 0;
        }

        size_t CalculateEvictionTarget(Importance priority) {
            // Intelligent eviction algorithm:
            // CRITICAL/ERROR: Never evict (protect critical messages)
            // WARN: Evict only 20% in emergency
            // INFO: Evict 40% when under pressure
            // DEBUG/TRACE: Evict aggressively (60-80%)

            switch (priority) {
                case Importance::CRITICAL:
                case Importance::ERROR:
                    return 0; // Never evict critical/error messages

                case Importance::WARN:
                    return std::max(static_cast<size_t>(1), priority_queues_[static_cast<size_t>(priority)].size() / 5); // 20%

                case Importance::INFO:
                    return std::max(static_cast<size_t>(1), priority_queues_[static_cast<size_t>(priority)].size() * 2 / 5); // 40%

                case Importance::DEBUG:
                case Importance::TRACE:
                default:
                    return std::max(static_cast<size_t>(2), priority_queues_[static_cast<size_t>(priority)].size() * 3 / 5); // 60%
            }
        }
    };

} // namespace QueueTesting

namespace ASFMLogger {
    namespace Core {
        namespace Testing {

            // =============================================================================
            // ULTRA-SPECIALIZED SMARTMESSAGEQUEUE TESTING
            // =============================================================================

            class SmartMessageQueueUltraTest : public ::testing::Test {
            protected:
                void SetUp() override {
                    // Create fresh queue for each test
                    queue_ = std::make_unique<QueueTesting::TestableSmartMessageQueue>(100);

                    // Initialize standard test messages with different priorities
                    using Imp = QueueTesting::Importance;
                    using Msg = QueueTesting::TestMessage;

                    standard_messages_ = {
                        Msg("Trace message", "TRACE", "Component1", Imp::TRACE, 1),
                        Msg("Debug diagnostic", "DEBUG", "Component2", Imp::DEBUG, 2),
                        Msg("Info message", "INFO", "Component3", Imp::INFO, 3),
                        Msg("Warning condition", "WARN", "Component4", Imp::WARN, 4),
                        Msg("Error occurred", "ERROR", "Component5", Imp::ERROR, 5),
                        Msg("Critical failure", "CRITICAL", "Component6", Imp::CRITICAL, 6)
                    };

                    // Generate bulk test data (100 messages with random priorities)
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<size_t> priority_dist(0, 5);

                    bulk_messages_.reserve(100);
                    for (size_t i = 0; i < 100; ++i) {
                        Imp random_priority = static_cast<Imp>(priority_dist(gen));
                        std::string msg = "Bulk message #" + std::to_string(i);
                        std::string level = "LEVEL" + std::to_string(i % 6);
                        std::string component = "BulkComponent" + std::to_string(i % 10);
                        bulk_messages_.emplace_back(msg, level, component, random_priority, 100 + i);
                    }
                }

                void TearDown() override {
                    queue_->Clear();
                    queue_.reset();
                }

                // Helper to verify priority order (CRITICAL first, then ERROR, etc.)
                bool VerifyPriorityOrder() {
                    std::vector<QueueTesting::TestMessage> dequeued_messages;

                    QueueTesting::TestMessage msg;
                    while (queue_->Dequeue(msg)) {
                        dequeued_messages.push_back(msg);
                    }

                    // Verify messages are dequeued in priority order (higher priority first)
                    for (size_t i = 1; i < dequeued_messages.size(); ++i) {
                        if (!(dequeued_messages[i-1].importance >= dequeued_messages[i].importance)) {
                            return false; // Priority order violated
                        }
                    }

                    return true;
                }

                // Helper to count messages by priority level
                std::vector<size_t> GetMessageCountByPriority(const std::vector<QueueTesting::TestMessage>& messages) {
                    std::vector<size_t> counts(6, 0); // 6 priority levels
                    for (const auto& msg : messages) {
                        counts[static_cast<size_t>(msg.importance)]++;
                    }
                    return counts;
                }

                std::unique_ptr<QueueTesting::TestableSmartMessageQueue> queue_;
                std::vector<QueueTesting::TestMessage> standard_messages_;
                std::vector<QueueTesting::TestMessage> bulk_messages_;
                using Importance = QueueTesting::Importance;
                using TestMessage = QueueTesting::TestMessage;
            };

            // =============================================================================
            // TASK 1.02A: SMARTMESSAGEQUEUE CORE FUNCTIONALITY TESTS
            // =============================================================================

            TEST_F(SmartMessageQueueUltraTest, TestQueueInitialization) {
                // Test queue initialization and capacity management

                auto queue_sizes = queue_->GetQueueSizes();
                ASSERT_EQ(queue_sizes.size(), 6); // Should have 6 priority levels

                // Verify all queues start empty
                for (size_t size : queue_sizes) {
                    ASSERT_EQ(size, 0);
                }

                ASSERT_EQ(queue_->GetSize(), 0);
                ASSERT_EQ(queue_->GetCapacity(), 100);
                ASSERT_TRUE(queue_->IsEmpty());
                ASSERT_FALSE(queue_->IsFull());

                // Test default capacity (1000)
                QueueTesting::TestableSmartMessageQueue default_queue;
                ASSERT_EQ(default_queue.GetCapacity(), 1000);
                ASSERT_TRUE(default_queue.IsEmpty());

                SUCCEED(); // Queue initialization validated
            }

            TEST_F(SmartMessageQueueUltraTest, TestPriorityEnqueueOperations) {
                // Test message enqueuing with priority preservation

                // Enqueue messages in reverse priority order
                for (auto it = standard_messages_.rbegin(); it != standard_messages_.rend(); ++it) {
                    ASSERT_TRUE(queue_->Enqueue(*it));
                }

                ASSERT_EQ(queue_->GetSize(), 6);
                ASSERT_FALSE(queue_->IsEmpty());

                // Verify correct distribution across priority queues
                auto queue_sizes = queue_->GetQueueSizes();
                for (size_t priority_level = 0; priority_level < queue_sizes.size(); ++priority_level) {
                    if (priority_level < standard_messages_.size()) {
                        ASSERT_EQ(queue_sizes[priority_level], 1); // Each priority level should have exactly 1 message
                    } else {
                        ASSERT_EQ(queue_sizes[priority_level], 0);
                    }
                }

                // Test duplicate priorities
                TestMessage duplicate1("Duplicate critical 1", "CRITICAL", "Comp", Importance::CRITICAL, 100);
                TestMessage duplicate2("Duplicate critical 2", "CRITICAL", "Comp", Importance::CRITICAL, 101);

                ASSERT_TRUE(queue_->Enqueue(duplicate1));
                ASSERT_TRUE(queue_->Enqueue(duplicate2));

                ASSERT_EQ(queue_->GetSize(), 8);

                auto updated_sizes = queue_->GetQueueSizes();
                ASSERT_EQ(updated_sizes[static_cast<size_t>(Importance::CRITICAL)], 3); // Original + 2 duplicates

                SUCCEED(); // Priority enqueue operations validated
            }

            TEST_F(SmartMessageQueueUltraTest, TestPriorityDequeueOperations) {
                // Test FIFO within priority levels and priority order across levels

                // Enqueue messages with mixed priorities
                for (const auto& msg : standard_messages_) {
                    ASSERT_TRUE(queue_->Enqueue(msg));
                }

                ASSERT_EQ(queue_->GetSize(), 6);

                // Dequeue and verify priority order (CRITICAL should come first)
                TestMessage dequeued;
                ASSERT_TRUE(queue_->Dequeue(dequeued));
                ASSERT_EQ(dequeued.importance, Importance::CRITICAL);
                ASSERT_EQ(dequeued.sequence_id, 6);

                ASSERT_TRUE(queue_->Dequeue(dequeued));
                ASSERT_EQ(dequeued.importance, Importance::ERROR);
                ASSERT_EQ(dequeued.sequence_id, 5);

                ASSERT_TRUE(queue_->Dequeue(dequeued));
                ASSERT_EQ(dequeued.importance, Importance::WARN);
                ASSERT_EQ(dequeued.sequence_id, 4);

                // Test dequeue from empty queue
                while (queue_->GetSize() > 0) {
                    ASSERT_TRUE(queue_->Dequeue(dequeued));
                }

                ASSERT_TRUE(queue_->IsEmpty());
                ASSERT_FALSE(queue_->Dequeue(dequeued)); // Should fail on empty queue

                SUCCEED(); // Priority dequeue operations validated
            }

            // =============================================================================
            // TASK 1.02B: SMARTMESSAGEQUEUE EVICTION ALGORITHM TESTS
            // =============================================================================

            TEST_F(SmartMessageQueueUltraTest, TestIntelligentEvictionAlgorithm) {
                // Test the core intelligent eviction logic that protects critical messages

                // Fill queue with different priority distributions
                std::vector<TestMessage> test_messages;
                size_t total_messages = 80; // Leave room for capacity tests

                // Create distribution: 5 CRITICAL, 5 ERROR, 10 WARN, 20 INFO, 20 DEBUG, 20 TRACE (80 total)
                std::vector<size_t> priority_counts = {5, 5, 10, 20, 20, 20};
                std::vector<Importance> priorities = {Importance::CRITICAL, Importance::ERROR, Importance::WARN,
                                                      Importance::INFO, Importance::DEBUG, Importance::TRACE};

                size_t seq_id = 1000;
                for (size_t p_idx = 0; p_idx < priorities.size(); ++p_idx) {
                    for (size_t count = 0; count < priority_counts[p_idx]; ++count) {
                        std::string msg = "Eviction test msg P" + std::to_string(p_idx) + " #" + std::to_string(count);
                        test_messages.emplace_back(msg, "INFO", "EvictionTest", priorities[p_idx], seq_id++);
                    }
                }

                // Enqueue all messages
                for (const auto& msg : test_messages) {
                    ASSERT_TRUE(queue_->Enqueue(msg));
                }

                ASSERT_EQ(queue_->GetSize(), 80);
                ASSERT_FALSE(queue_->IsFull());

                // Verify initial priority distribution
                auto initial_sizes = queue_->GetQueueSizes();
                for (size_t p_idx = 0; p_idx < priorities.size(); ++p_idx) {
                    size_t expected_count = priority_counts[p_idx];
                    size_t actual_count = initial_sizes[p_idx];
                    ASSERT_EQ(actual_count, expected_count) << "Priority " << p_idx << " initial count mismatch";
                }

                // Now force eviction by filling to capacity and beyond
                size_t remaining_capacity = queue_->GetCapacity() - queue_->GetSize(); // 20 messages

                // Add messages that should trigger eviction (lower priorities get evicted)
                for (size_t i = 0; i < remaining_capacity; ++i) {
                    TestMessage filler_msg("Filler #" + std::to_string(i), "INFO", "Filler", Importance::TRACE, seq_id++);
                    ASSERT_TRUE(queue_->EnqueueWithEviction(filler_msg));
                }

                // Should be at capacity now
                ASSERT_EQ(queue_->GetSize(), queue_->GetCapacity());
                ASSERT_TRUE(queue_->IsFull());

                // Test adding one more message - should trigger intelligent eviction
                TestMessage over_limit_msg("Over limit", "INFO", "Overflow", Importance::TRACE, seq_id++);
                ASSERT_TRUE(queue_->EnqueueWithEviction(over_limit_msg));

                // Size should still be at capacity after intelligent eviction
                ASSERT_EQ(queue_->GetSize(), queue_->GetCapacity());

                // Verify critical/error messages are preserved (should not be evicted)
                auto post_eviction_sizes = queue_->GetQueueSizes();
                ASSERT_EQ(post_eviction_sizes[static_cast<size_t>(Importance::CRITICAL)], 5) << "CRITICAL messages should never be evicted";
                ASSERT_EQ(post_eviction_sizes[static_cast<size_t>(Importance::ERROR)], 5) << "ERROR messages should never be evicted";

                // Lower priority messages should be reduced
                ASSERT_LT(post_eviction_sizes[static_cast<size_t>(Importance::TRACE)], 20) << "TRACE messages should be significantly reduced";

                SUCCEED(); // Intelligent eviction algorithm validated
            }

            TEST_F(SmartMessageQueueUltraTest, TestCapacityManagement) {
                // Test queue capacity limits and boundary conditions

                const size_t capacity = queue_->GetCapacity();
                ASSERT_EQ(capacity, 100);

                // Test precisely filling to capacity
                std::vector<TestMessage> fill_messages;
                for (size_t i = 0; i < capacity; ++i) {
                    fill_messages.emplace_back("Capacity test #" + std::to_string(i), "INFO", "CapacityTest", Importance::INFO, i);
                }

                for (size_t i = 0; i < capacity; ++i) {
                    ASSERT_TRUE(queue_->Enqueue(fill_messages[i])) << "Failed to enqueue message " << i;
                }

                ASSERT_EQ(queue_->GetSize(), capacity);
                ASSERT_TRUE(queue_->IsFull());
                ASSERT_FALSE(queue_->IsEmpty());

                // Test enqueue beyond capacity without eviction
                TestMessage overflow_msg("Overflow without eviction", "ERROR", "Overflow", Importance::ERROR, 999);
                ASSERT_FALSE(queue_->Enqueue(overflow_msg)) << "Should fail to enqueue beyond capacity";
                ASSERT_EQ(queue_->GetSize(), capacity); // Size unchanged

                // Test enqueue with eviction (should succeed)
                ASSERT_TRUE(queue_->EnqueueWithEviction(overflow_msg));
                ASSERT_EQ(queue_->GetSize(), capacity); // Size still at capacity after eviction

                // Clear and test empty queue behavior
                queue_->Clear();
                ASSERT_TRUE(queue_->IsEmpty());
                ASSERT_EQ(queue_->GetSize(), 0);

                // Test zero capacity queue (edge case)
                QueueTesting::TestableSmartMessageQueue zero_queue(0);
                ASSERT_EQ(zero_queue.GetCapacity(), 0);
                ASSERT_TRUE(zero_queue.IsEmpty());
                ASSERT_TRUE(zero_queue.IsFull()); // Zero capacity is always "full"

                TestMessage zero_test_msg("Zero capacity test", "INFO", "ZeroTest", Importance::CRITICAL, 1);
                ASSERT_FALSE(zero_queue.Enqueue(zero_test_msg));
                ASSERT_FALSE(zero_queue.EnqueueWithEviction(zero_test_msg));

                SUCCEED(); // Capacity management validated
            }

            // =============================================================================
            // TASK 1.02C: SMARTMESSAGEQUEUE PERFORMANCE & SCALING TESTS
            // =============================================================================

            TEST_F(SmartMessageQueueUltraTest, TestConcurrencyStressTesting) {
                // Test thread safety and concurrent access under high load

                const size_t NUM_PRODUCER_THREADS = 4;
                const size_t NUM_CONSUMER_THREADS = 2;
                const size_t MESSAGES_PER_PRODUCER = 1000;
                const size_t TOTAL_MESSAGES = NUM_PRODUCER_THREADS * MESSAGES_PER_PRODUCER;

                std::vector<std::thread> producer_threads;
                std::vector<std::thread> consumer_threads;

                std::atomic<size_t> messages_produced{0};
                std::atomic<size_t> messages_consumed{0};
                std::atomic<bool> producers_done{false};
                std::mutex cout_mutex;

                // Producer function
                auto producer_function = [&](size_t producer_id) {
                    size_t base_id = producer_id * MESSAGES_PER_PRODUCER;
                    for (size_t i = 0; i < MESSAGES_PER_PRODUCER; ++i) {
                        // Create message with varying priorities for realistic testing
                        Importance priority = static_cast<Importance>((i % 6));
                        std::string msg = "Producer " + std::to_string(producer_id) + " message " + std::to_string(i);
                        std::string level = "LEVEL" + std::to_string(i % 6);
                        std::string component = "ProdComp" + std::to_string(producer_id);
                        size_t seq_id = base_id + i;

                        TestMessage message(msg, level, component, priority, seq_id);

                        // Retry logic for full queue scenarios
                        bool enqueued = false;
                        while (!enqueued) {
                            enqueued = queue_->EnqueueWithEviction(message);
                            if (!enqueued) {
                                std::this_thread::sleep_for(std::chrono::microseconds(100)); // Brief pause before retry
                            }
                        }

                        messages_produced++;
                    }

                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "Producer " << producer_id << " completed: " << MESSAGES_PER_PRODUCER << " messages\n";
                    }
                };

                // Consumer function
                auto consumer_function = [&](size_t consumer_id) {
                    size_t consumed_count = 0;
                    TestMessage message;

                    while (!producers_done || !queue_->IsEmpty()) {
                        if (queue_->Dequeue(message)) {
                            consumed_count++;
                            messages_consumed++;

                            // Brief processing simulation
                            std::this_thread::sleep_for(std::chrono::microseconds(10));
                        } else {
                            // No messages available, brief pause
                            std::this_thread::sleep_for(std::chrono::microseconds(50));
                        }
                    }

                    {
                        std::lock_guard<std::mutex> lock(cout_mutex);
                        std::cout << "Consumer " << consumer_id << " completed: " << consumed_count << " messages\n";
                    }
                };

                // Launch producer threads
                for (size_t p = 0; p < NUM_PRODUCER_THREADS; ++p) {
                    producer_threads.emplace_back(producer_function, p);
                }

                // Launch consumer threads
                for (size_t c = 0; c < NUM_CONSUMER_THREADS; ++c) {
                    consumer_threads.emplace_back(consumer_function, c);
                }

                // Wait for producers to complete
                for (auto& thread : producer_threads) {
                    thread.join();
                }

                producers_done = true;

                // Wait for consumers to complete processing remaining messages
                for (auto& thread : consumer_threads) {
                    thread.join();
                }

                // Verify all messages were processed
                ASSERT_EQ(messages_produced.load(), TOTAL_MESSAGES);
                ASSERT_EQ(messages_consumed.load(), TOTAL_MESSAGES);

                std::cout << "\nConcurrency Stress Test Results:" << std::endl;
                std::cout << "  Producers: " << NUM_PRODUCER_THREADS << " threads" << std::endl;
                std::cout << "  Consumers: " << NUM_CONSUMER_THREADS << " threads" << std::endl;
                std::cout << "  Messages per producer: " << MESSAGES_PER_PRODUCER << std::endl;
                std::cout << "  Total messages: " << TOTAL_MESSAGES << std::endl;
                std::cout << "  Messages produced: " << messages_produced.load() << std::endl;
                std::cout << "  Messages consumed: " << messages_consumed.load() << std::endl;
                std::cout << "  Final queue size: " << queue_->GetSize() << std::endl;

                SUCCEED(); // Concurrency stress testing validated
            }

            TEST_F(SmartMessageQueueUltraTest, TestPerformanceBenchmarking) {
                // Test performance characteristics under various loads and conditions

                // Performance test parameters
                const size_t WARMUP_ITERATIONS = 1000;
                const size_t ENQUEUE_PERFORMANCE_ITERATIONS = 50000;
                const size_t DEQUEUE_PERFORMANCE_ITERATIONS = 25000;

                // Warmup phase
                for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
                    TestMessage warmup_msg("Warmup " + std::to_string(i), "DEBUG", "Warmup", Importance::DEBUG, 2000 + i);
                    queue_->EnqueueWithEviction(warmup_msg);
                }
                queue_->Clear(); // Clear warmup data

                // Standard performance test (enqueue)
                auto enqueue_start = std::chrono::steady_clock::now();
                size_t enqueued_count = 0;
                for (size_t i = 0; i < ENQUEUE_PERFORMANCE_ITERATIONS; ++i) {
                    TestMessage msg("Performance message " + std::to_string(i), "INFO", "PerformanceTest", Importance::INFO, i);
                    if (queue_->EnqueueWithEviction(msg)) {
                        enqueued_count++;
                    }
                }
                auto enqueue_end = std::chrono::steady_clock::now();
                auto enqueue_duration = std::chrono::duration_cast<std::chrono::milliseconds>(enqueue_end - enqueue_start);

                // Performance test (dequeue)
                auto dequeue_start = std::chrono::steady_clock::now();
                size_t dequeued_count = 0;
                TestMessage dummy_msg;
                for (size_t i = 0; i < DEQUEUE_PERFORMANCE_ITERATIONS && queue_->Dequeue(dummy_msg); ++i) {
                    dequeued_count++;
                }
                auto dequeue_end = std::chrono::steady_clock::now();
                auto dequeue_duration = std::chrono::duration_cast<std::chrono::milliseconds>(dequeue_end - dequeue_start);

                // Calculate performance metrics
                double enqueue_avg_time = (enqueued_count > 0) ? static_cast<double>(enqueue_duration.count()) / enqueued_count : 0.0;
                double enqueue_ops_per_second = (enqueue_avg_time > 0.0) ? (1000.0 / enqueue_avg_time) : 0.0;
                double dequeue_avg_time = (dequeued_count > 0) ? static_cast<double>(dequeue_duration.count()) / dequeued_count : 0.0;
                double dequeue_ops_per_second = (dequeue_avg_time > 0.0) ? (1000.0 / dequeue_avg_time) : 0.0;

                // Enterprise requirements
                ASSERT_LT(enqueue_avg_time, 0.1);     // Under 100 microseconds per enqueue
                ASSERT_GT(enqueue_ops_per_second, 10000);  // 10,000+ enqueues per second
                ASSERT_LT(dequeue_avg_time, 0.05);     // Under 50 microseconds per dequeue
                ASSERT_GT(dequeue_ops_per_second, 20000);  // 20,000+ dequeues per second

                std::cout << "\nSmartMessageQueue Performance Results:" << std::endl;
                std::cout << "  Enqueue Test (" << ENQUEUE_PERFORMANCE_ITERATIONS << " operations): " << enqueue_duration.count() << "ms" << std::endl;
                std::cout << "    Average enqueue time: " << enqueue_avg_time << "ms" << std::endl;
                std::cout << "    Enqueues per second: " << enqueue_ops_per_second << std::endl;
                std::cout << "    Messages enqueued: " << enqueued_count << std::endl;

                std::cout << "  Dequeue Test (" << dequeued_count << " operations): " << dequeue_duration.count() << "ms" << std::endl;
                std::cout << "    Average dequeue time: " << dequeue_avg_time << "ms" << std::endl;
                std::cout << "    Dequeues per second: " << dequeue_ops_per_second << std::endl;
                std::cout << "    Messages dequeued: " << dequeued_count << std::endl;

                // Test concurrent performance (simple multi-threaded test)
                const size_t NUM_PARALLEL_THREADS = 4;
                const size_t MESSAGES_PER_THREAD = 10000;

                std::vector<std::thread> perf_threads;
                std::atomic<size_t> parallel_operations{0};

                auto parallel_worker = [&]() {
                    size_t local_ops = 0;
                    for (size_t i = 0; i < MESSAGES_PER_THREAD; ++i) {
                        TestMessage msg("Parallel msg " + std::to_string(i), "INFO", "Parallel", Importance::INFO, i);
                        if (queue_->EnqueueWithEviction(msg)) {
                            local_ops++;
                        }
                        TestMessage dummy;
                        if (queue_->Dequeue(dummy)) {
                            local_ops++;
                        }
                    }
                    parallel_operations += local_ops;
                };

                queue_->Clear(); // Clear for parallel test

                auto parallel_start = std::chrono::steady_clock::now();
                for (size_t t = 0; t < NUM_PARALLEL_THREADS; ++t) {
                    perf_threads.emplace_back(parallel_worker);
                }

                for (auto& thread : perf_threads) {
                    thread.join();
                }
                auto parallel_end = std::chrono::steady_clock::now();
                auto parallel_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parallel_end - parallel_start);

                double parallel_ops_per_second = static_cast<double>(parallel_operations.load()) * 1000.0 / parallel_duration.count();
                double avg_parallel_time_per_op = static_cast<double>(parallel_duration.count()) / parallel_operations.load();

                std::cout << "  Parallel Test (" << NUM_PARALLEL_THREADS << " threads): " << parallel_duration.count() << "ms" << std::endl;
                std::cout << "    Total operations: " << parallel_operations.load() << std::endl;
                std::cout << "    Average time per operation: " << avg_parallel_time_per_op << "ms" << std::endl;
                std::cout << "    Operations per second: " << parallel_ops_per_second << std::endl;

                // Enterprise parallel performance requirements
                ASSERT_GT(parallel_ops_per_second, 25000);   // 25,000+ operations per second in parallel
                ASSERT_LT(avg_parallel_time_per_op, 0.2);    // Under 200 microseconds average

                SUCCEED(); // Performance benchmarking validated
            }

        } // namespace Testing
    } // namespace Core
} // namespace ASFMLogger
