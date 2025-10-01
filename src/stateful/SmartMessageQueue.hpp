#ifndef __ASFM_LOGGER_SMART_MESSAGE_QUEUE_HPP__
#define __ASFM_LOGGER_SMART_MESSAGE_QUEUE_HPP__

/**
 * ASFMLogger Stateful Smart Message Queue
 *
 * Stateful wrapper class for intelligent message queuing using toolbox internally.
 * Provides object-oriented interface for smart buffering and prioritization.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/SmartQueueConfiguration.hpp"
#include "structs/PersistencePolicy.hpp"
#include "toolbox/SmartQueueToolbox.hpp"
#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

class SmartMessageQueue {
private:
    uint32_t queue_id_;
    SmartQueueConfiguration config_;
    std::deque<std::pair<LogMessageData, QueueEntryMetadata>> message_queue_;
    std::unordered_map<MessageImportance, size_t> space_usage_by_importance_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_condition_;

    // Performance tracking
    std::atomic<uint64_t> total_messages_queued_;
    std::atomic<uint64_t> total_messages_dequeued_;
    std::atomic<uint64_t> total_messages_evicted_;
    std::atomic<size_t> current_memory_usage_;

    // Priority preservation
    std::vector<QueueEntryMetadata> preserved_messages_;
    std::mutex preservation_mutex_;

    // Batch processing
    std::vector<LogMessageData> current_batch_;
    std::mutex batch_mutex_;
    DWORD last_batch_time_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    SmartMessageQueue();

    /**
     * @brief Constructor with configuration
     * @param config Queue configuration
     */
    explicit SmartMessageQueue(const SmartQueueConfiguration& config);

    /**
     * @brief Constructor with queue name and application
     * @param queue_name Name for the queue
     * @param application_name Application name
     * @param max_size Maximum queue size (optional)
     */
    SmartMessageQueue(const std::string& queue_name,
                     const std::string& application_name,
                     size_t max_size = 10000);

    /**
     * @brief Copy constructor
     * @param other Queue to copy from
     */
    SmartMessageQueue(const SmartMessageQueue& other);

    /**
     * @brief Move constructor
     * @param other Queue to move from
     */
    SmartMessageQueue(SmartMessageQueue&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Queue to assign from
     * @return Reference to this queue
     */
    SmartMessageQueue& operator=(const SmartMessageQueue& other);

    /**
     * @brief Move assignment operator
     * @param other Queue to move from
     * @return Reference to this queue
     */
    SmartMessageQueue& operator=(SmartMessageQueue&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~SmartMessageQueue();

    // =================================================================================
    // QUEUE MANAGEMENT
    // =================================================================================

    /**
     * @brief Configure the queue
     * @param config New queue configuration
     * @return true if configuration is valid and was applied
     */
    bool configure(const SmartQueueConfiguration& config);

    /**
     * @brief Get current queue configuration
     * @return Current configuration
     */
    SmartQueueConfiguration getConfiguration() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return config_;
    }

    /**
     * @brief Get queue ID
     * @return Queue identifier
     */
    uint32_t getId() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return queue_id_;
    }

    /**
     * @brief Get queue name
     * @return Queue name
     */
    std::string getName() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return std::string(config_.queue_name);
    }

    // =================================================================================
    // MESSAGE OPERATIONS
    // =================================================================================

    /**
     * @brief Add message to queue with intelligent prioritization
     * @param message Message to add
     * @param context Persistence context for priority calculation
     * @return true if message was added successfully
     */
    bool enqueue(const LogMessageData& message, const PersistenceDecisionContext& context);

    /**
     * @brief Add message with automatic priority calculation
     * @param message Message to add
     * @return true if message was added successfully
     */
    bool enqueue(const LogMessageData& message);

    /**
     * @brief Remove and return next message from queue
     * @param message Output parameter for dequeued message
     * @param metadata Output parameter for message metadata
     * @return true if message was dequeued successfully
     */
    bool dequeue(LogMessageData& message, QueueEntryMetadata& metadata);

    /**
     * @brief Peek at next message without removing it
     * @param message Output parameter for message
     * @param metadata Output parameter for metadata
     * @return true if peek was successful
     */
    bool peek(LogMessageData& message, QueueEntryMetadata& metadata) const;

    /**
     * @brief Try to dequeue message with timeout
     * @param message Output parameter for message
     * @param metadata Output parameter for metadata
     * @param timeout_ms Timeout in milliseconds
     * @return true if message was dequeued successfully
     */
    bool dequeueWithTimeout(LogMessageData& message, QueueEntryMetadata& metadata, DWORD timeout_ms);

    // =================================================================================
    // BATCH OPERATIONS
    // =================================================================================

    /**
     * @brief Dequeue multiple messages as a batch
     * @param messages Vector to store dequeued messages
     * @param max_count Maximum number of messages to dequeue
     * @return Number of messages actually dequeued
     */
    size_t dequeueBatch(std::vector<LogMessageData>& messages, size_t max_count);

    /**
     * @brief Dequeue messages for persistence processing
     * @param messages Vector to store messages for persistence
     * @param max_count Maximum number of messages
     * @param context Context for persistence decisions
     * @return Number of messages dequeued for persistence
     */
    size_t dequeueForPersistence(std::vector<LogMessageData>& messages,
                                size_t max_count,
                                const PersistenceDecisionContext& context);

    /**
     * @brief Create optimal batch for current queue state
     * @param max_batch_size Maximum batch size
     * @return Batch configuration for optimal processing
     */
    QueueBatch createOptimalBatch(size_t max_batch_size);

    /**
     * @brief Create priority batch for urgent processing
     * @return Priority batch with most important messages
     */
    QueueBatch createPriorityBatch();

    // =================================================================================
    // PRIORITY PRESERVATION
    // =================================================================================

    /**
     * @brief Mark high-priority messages for preservation
     * @param count Number of messages to preserve
     * @param duration_ms How long to preserve them
     * @return Number of messages marked for preservation
     */
    size_t preservePriorityMessages(size_t count, DWORD duration_ms);

    /**
     * @brief Extend preservation for existing preserved messages
     * @param additional_duration_ms Additional preservation time
     * @return Number of messages with extended preservation
     */
    size_t extendPriorityPreservation(DWORD additional_duration_ms);

    /**
     * @brief Get number of preserved messages
     * @return Number of messages currently preserved
     */
    size_t getPreservedMessageCount() const {
        std::lock_guard<std::mutex> lock(preservation_mutex_);
        return preserved_messages_.size();
    }

    /**
     * @brief Clear all preserved message status
     * @return Number of messages affected
     */
    size_t clearPreservedMessages();

    // =================================================================================
    // EVICTION MANAGEMENT
    // =================================================================================

    /**
     * @brief Perform intelligent eviction to free space
     * @param space_needed_bytes Space needed to free
     * @return Number of bytes freed
     */
    size_t performIntelligentEviction(size_t space_needed_bytes);

    /**
     * @brief Perform age-based eviction
     * @param max_age_seconds Maximum age for eviction
     * @return Number of messages evicted
     */
    size_t performAgeBasedEviction(DWORD max_age_seconds);

    /**
     * @brief Perform importance-based eviction
     * @param min_importance Minimum importance to keep
     * @return Number of messages evicted
     */
    size_t performImportanceBasedEviction(MessageImportance min_importance);

    /**
     * @brief Force eviction to reduce queue size
     * @param target_size Target queue size after eviction
     * @return Number of messages evicted
     */
    size_t forceEvictionToSize(size_t target_size);

    /**
     * @brief Get eviction candidates for external evaluation
     * @param count Number of candidates to return
     * @return Vector of eviction candidates
     */
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> getEvictionCandidates(size_t count);

    // =================================================================================
    // QUEUE INSPECTION
    // =================================================================================

    /**
     * @brief Get current queue size
     * @return Number of messages in queue
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return message_queue_.size();
    }

    /**
     * @brief Get current memory usage
     * @return Memory usage in bytes
     */
    size_t getMemoryUsage() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return current_memory_usage_;
    }

    /**
     * @brief Check if queue is empty
     * @return true if queue is empty
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return message_queue_.empty();
    }

    /**
     * @brief Check if queue is full
     * @return true if queue is at maximum capacity
     */
    bool isFull() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return message_queue_.size() >= config_.max_total_size;
    }

    /**
     * @brief Get space usage by importance level
     * @return Map of importance level to space usage
     */
    std::unordered_map<MessageImportance, size_t> getSpaceUsageByImportance() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return space_usage_by_importance_;
    }

    /**
     * @brief Get messages by importance level
     * @param importance Importance level to filter by
     * @return Vector of messages with specified importance
     */
    std::vector<LogMessageData> getMessagesByImportance(MessageImportance importance) const;

    /**
     * @brief Get oldest message in queue
     * @param message Output parameter for oldest message
     * @param metadata Output parameter for metadata
     * @return true if oldest message was found
     */
    bool getOldestMessage(LogMessageData& message, QueueEntryMetadata& metadata) const;

    /**
     * @brief Get highest priority message in queue
     * @param message Output parameter for priority message
     * @param metadata Output parameter for metadata
     * @return true if priority message was found
     */
    bool getHighestPriorityMessage(LogMessageData& message, QueueEntryMetadata& metadata) const;

    // =================================================================================
    // STATISTICS AND MONITORING
    // =================================================================================

    /**
     * @brief Get queue statistics
     * @return Current queue statistics
     */
    SmartQueueStatistics getStatistics() const;

    /**
     * @brief Reset queue statistics
     */
    void resetStatistics();

    /**
     * @brief Get queue performance metrics
     * @return Vector of performance metrics
     */
    std::vector<std::pair<std::string, double>> getPerformanceMetrics() const;

    /**
     * @brief Get total messages queued
     * @return Total messages that have been queued
     */
    uint64_t getTotalQueued() const {
        return total_messages_queued_;
    }

    /**
     * @brief Get total messages dequeued
     * @return Total messages that have been dequeued
     */
    uint64_t getTotalDequeued() const {
        return total_messages_dequeued_;
    }

    /**
     * @brief Get total messages evicted
     * @return Total messages that have been evicted
     */
    uint64_t getTotalEvicted() const {
        return total_messages_evicted_;
    }

    /**
     * @brief Get average queue latency
     * @return Average time messages spend in queue (milliseconds)
     */
    DWORD getAverageQueueLatency() const;

    // =================================================================================
    // QUEUE MAINTENANCE
    // =================================================================================

    /**
     * @brief Clean up expired messages
     * @return Number of messages removed
     */
    size_t cleanupExpiredMessages();

    /**
     * @brief Compact queue to optimize memory usage
     * @return true if compaction was successful
     */
    bool compact();

    /**
     * @brief Clear all messages from queue
     * @return Number of messages removed
     */
    size_t clear();

    /**
     * @brief Resize queue capacity
     * @param new_max_size New maximum size
     * @return true if resize was successful
     */
    bool resize(size_t new_max_size);

    /**
     * @brief Validate queue integrity
     * @return true if queue is in valid state
     */
    bool validate() const;

    // =================================================================================
    // ADVANCED OPERATIONS
    // =================================================================================

    /**
     * @brief Reorder queue by priority
     * @return true if reorder was successful
     */
    bool reorderByPriority();

    /**
     * @brief Move message to front of queue (for high priority)
     * @param message_id Message ID to move
     * @return true if message was found and moved
     */
    bool moveToFront(uint32_t message_id);

    /**
     * @brief Move message to back of queue (for low priority)
     * @param message_id Message ID to move
     * @return true if message was found and moved
     */
    bool moveToBack(uint32_t message_id);

    /**
     * @brief Find message by ID
     * @param message_id Message ID to find
     * @param message Output parameter for message
     * @param metadata Output parameter for metadata
     * @return true if message was found
     */
    bool findMessage(uint32_t message_id, LogMessageData& message, QueueEntryMetadata& metadata) const;

    /**
     * @brief Remove specific message by ID
     * @param message_id Message ID to remove
     * @return true if message was found and removed
     */
    bool removeMessage(uint32_t message_id);

    // =================================================================================
    // SYNCHRONIZATION
    // =================================================================================

    /**
     * @brief Wait for queue to have messages
     * @param timeout_ms Timeout in milliseconds
     * @return true if queue has messages or timeout occurred
     */
    bool waitForMessages(DWORD timeout_ms);

    /**
     * @brief Notify waiting threads that messages are available
     */
    void notifyMessageAvailable();

    /**
     * @brief Get approximate queue size (lock-free)
     * @return Approximate number of messages in queue
     */
    size_t getApproximateSize() const {
        return message_queue_.size();  // Approximate, not locked
    }

private:
    // Private helper methods
    void initializeWithConfiguration(const SmartQueueConfiguration& config);
    bool canAcceptMessage(const LogMessageData& message, const PersistenceDecisionContext& context);
    void updateSpaceUsage(const LogMessageData& message, MessageImportance importance, bool is_adding);
    void updateQueueStatistics(const std::string& operation, size_t message_count, size_t bytes_affected);
    size_t calculateMessageSpace(const LogMessageData& message) const;
    MessageImportance determineMessageImportance(const LogMessageData& message,
                                                const PersistenceDecisionContext& context);
    DWORD calculateCurrentTimestamp() const;
    void maintainQueueOrder();
    bool isSpaceAvailable(const LogMessageData& message, MessageImportance importance);
};

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

/**
 * @brief Create smart queue for application
 * @param application_name Name of the application
 * @param queue_name Name for the queue
 * @param max_size Maximum queue size
 * @return New SmartMessageQueue for the application
 */
SmartMessageQueue CreateApplicationSmartQueue(const std::string& application_name,
                                             const std::string& queue_name,
                                             size_t max_size = 10000);

/**
 * @brief Output stream operator for SmartMessageQueue
 * @param os Output stream
 * @param queue Queue to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& os, const SmartMessageQueue& queue);

#endif // __ASFM_LOGGER_SMART_MESSAGE_QUEUE_HPP__