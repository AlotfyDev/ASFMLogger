#ifndef __ASFM_LOGGER_MESSAGE_QUEUE_MANAGER_HPP__
#define __ASFM_LOGGER_MESSAGE_QUEUE_MANAGER_HPP__

/**
 * ASFMLogger Message Queue Manager
 *
 * Stateful manager class for coordinating multiple smart message queues.
 * Uses SmartQueueToolbox internally for all queue operations.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/SmartQueueConfiguration.hpp"
#include "stateful/SmartMessageQueue.hpp"
#include "toolbox/SmartQueueToolbox.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

class MessageQueueManager {
private:
    std::unordered_map<uint32_t, std::unique_ptr<SmartMessageQueue>> queues_;
    std::unordered_map<std::string, std::vector<uint32_t>> application_queues_;
    mutable std::mutex manager_mutex_;

    // Global statistics
    std::atomic<uint64_t> total_queues_created_;
    std::atomic<uint64_t> total_messages_processed_;
    std::atomic<uint64_t> total_evictions_performed_;

    // Performance monitoring
    DWORD last_performance_check_;
    std::vector<DWORD> recent_throughput_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    MessageQueueManager();

    /**
     * @brief Destructor
     */
    ~MessageQueueManager();

    // =================================================================================
    // QUEUE CREATION AND MANAGEMENT
    // =================================================================================

    /**
     * @brief Create new smart queue for an application
     * @param application_name Name of the application
     * @param queue_name Name for the queue
     * @param max_size Maximum queue size (optional)
     * @return Queue ID of the created queue
     */
    uint32_t createQueue(const std::string& application_name,
                        const std::string& queue_name,
                        size_t max_size = 10000);

    /**
     * @brief Create queue with specific configuration
     * @param application_name Name of the application
     * @param queue_name Name for the queue
     * @param config Queue configuration
     * @return Queue ID of the created queue
     */
    uint32_t createQueueWithConfig(const std::string& application_name,
                                  const std::string& queue_name,
                                  const SmartQueueConfiguration& config);

    /**
     * @brief Get queue by ID
     * @param queue_id Queue ID to retrieve
     * @return Pointer to queue or nullptr if not found
     */
    SmartMessageQueue* getQueue(uint32_t queue_id);

    /**
     * @brief Get const pointer to queue by ID
     * @param queue_id Queue ID to retrieve
     * @return Const pointer to queue or nullptr if not found
     */
    const SmartMessageQueue* getQueue(uint32_t queue_id) const;

    /**
     * @brief Get all queues for an application
     * @param application_name Application name
     * @return Vector of queue pointers
     */
    std::vector<SmartMessageQueue*> getApplicationQueues(const std::string& application_name);

    /**
     * @brief Get all queues for an application (const)
     * @param application_name Application name
     * @return Vector of const queue pointers
     */
    std::vector<const SmartMessageQueue*> getApplicationQueues(const std::string& application_name) const;

    /**
     * @brief Remove queue by ID
     * @param queue_id Queue ID to remove
     * @return true if queue was found and removed
     */
    bool removeQueue(uint32_t queue_id);

    /**
     * @brief Remove all queues for an application
     * @param application_name Application name
     * @return Number of queues removed
     */
    size_t removeApplicationQueues(const std::string& application_name);

    // =================================================================================
    // MESSAGE DISTRIBUTION
    // =================================================================================

    /**
     * @brief Distribute message to appropriate application queue
     * @param message Message to distribute
     * @param application_name Target application name
     * @param context Persistence context for priority calculation
     * @return true if message was queued successfully
     */
    bool distributeMessage(const LogMessageData& message,
                          const std::string& application_name,
                          const PersistenceDecisionContext& context);

    /**
     * @brief Distribute message using automatic application detection
     * @param message Message to distribute
     * @param context Persistence context
     * @return true if message was queued successfully
     */
    bool distributeMessage(const LogMessageData& message, const PersistenceDecisionContext& context);

    /**
     * @brief Distribute multiple messages to appropriate queues
     * @param messages Vector of messages to distribute
     * @param context Persistence context
     * @return Number of messages successfully queued
     */
    size_t distributeMessages(const std::vector<LogMessageData>& messages,
                             const PersistenceDecisionContext& context);

    /**
     * @brief Get best queue for message based on current conditions
     * @param message Message to evaluate
     * @param application_name Application name
     * @return Queue ID of best queue or 0 if no suitable queue found
     */
    uint32_t getBestQueueForMessage(const LogMessageData& message, const std::string& application_name);

    // =================================================================================
    // BATCH PROCESSING
    // =================================================================================

    /**
     * @brief Process messages from all queues for persistence
     * @param messages Vector to store messages for persistence
     * @param max_messages Maximum messages to retrieve
     * @param context Persistence context
     * @return Number of messages retrieved
     */
    size_t processQueuesForPersistence(std::vector<LogMessageData>& messages,
                                      size_t max_messages,
                                      const PersistenceDecisionContext& context);

    /**
     * @brief Process priority messages from all queues
     * @param messages Vector to store priority messages
     * @param max_messages Maximum priority messages to retrieve
     * @return Number of priority messages retrieved
     */
    size_t processPriorityMessages(std::vector<LogMessageData>& messages, size_t max_messages);

    /**
     * @brief Process messages from specific application queues
     * @param application_name Application name
     * @param messages Vector to store messages
     * @param max_messages Maximum messages to retrieve
     * @param context Persistence context
     * @return Number of messages retrieved
     */
    size_t processApplicationQueues(const std::string& application_name,
                                   std::vector<LogMessageData>& messages,
                                   size_t max_messages,
                                   const PersistenceDecisionContext& context);

    // =================================================================================
    // QUEUE MONITORING AND MAINTENANCE
    // =================================================================================

    /**
     * @brief Get all queue IDs
     * @return Vector of all queue IDs
     */
    std::vector<uint32_t> getAllQueueIds() const;

    /**
     * @brief Get queue IDs for an application
     * @param application_name Application name
     * @return Vector of queue IDs for the application
     */
    std::vector<uint32_t> getApplicationQueueIds(const std::string& application_name) const;

    /**
     * @brief Get total number of queues
     * @return Total queue count
     */
    size_t getQueueCount() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return queues_.size();
    }

    /**
     * @brief Get total number of queues for an application
     * @param application_name Application name
     * @return Queue count for the application
     */
    size_t getApplicationQueueCount(const std::string& application_name) const;

    /**
     * @brief Check if queue exists
     * @param queue_id Queue ID to check
     * @return true if queue exists
     */
    bool hasQueue(uint32_t queue_id) const;

    /**
     * @brief Check if application has queues
     * @param application_name Application name to check
     * @return true if application has queues
     */
    bool hasApplicationQueues(const std::string& application_name) const;

    // =================================================================================
    // PERFORMANCE MONITORING
    // =================================================================================

    /**
     * @brief Get manager statistics
     * @return String containing manager statistics
     */
    std::string getManagerStatistics() const;

    /**
     * @brief Get performance metrics for all queues
     * @return Vector of performance metrics
     */
    std::vector<std::pair<std::string, double>> getAllQueuesPerformanceMetrics() const;

    /**
     * @brief Get throughput statistics
     * @return String containing throughput information
     */
    std::string getThroughputStatistics() const;

    /**
     * @brief Get total messages processed
     * @return Total messages processed across all queues
     */
    uint64_t getTotalMessagesProcessed() const {
        return total_messages_processed_;
    }

    /**
     * @brief Get total evictions performed
     * @return Total evictions across all queues
     */
    uint64_t getTotalEvictionsPerformed() const {
        return total_evictions_performed_;
    }

    /**
     * @brief Get total queues created
     * @return Total queues created by this manager
     */
    uint64_t getTotalQueuesCreated() const {
        return total_queues_created_;
    }

    /**
     * @brief Perform maintenance on all queues
     * @return Number of queues that performed maintenance
     */
    size_t performMaintenance();

    /**
     * @brief Optimize all queues for current conditions
     * @return Number of queues that were optimized
     */
    size_t optimizeAllQueues();

    // =================================================================================
    // LOAD BALANCING
    // =================================================================================

    /**
     * @brief Balance load across application queues
     * @param application_name Application name
     * @return true if load balancing was performed
     */
    bool balanceApplicationQueues(const std::string& application_name);

    /**
     * @brief Redistribute messages from overloaded queues
     * @param threshold_usage_percent Usage threshold for redistribution (0-100)
     * @return Number of messages redistributed
     */
    size_t redistributeOverloadedQueues(DWORD threshold_usage_percent = 80);

    /**
     * @brief Get queue with most available space for an application
     * @param application_name Application name
     * @return Queue ID with most available space or 0 if no queues available
     */
    uint32_t getQueueWithMostSpace(const std::string& application_name);

    /**
     * @brief Get queue with least load for an application
     * @param application_name Application name
     * @return Queue ID with least load or 0 if no queues available
     */
    uint32_t getQueueWithLeastLoad(const std::string& application_name);

    // =================================================================================
    // EMERGENCY OPERATIONS
    // =================================================================================

    /**
     * @brief Handle emergency mode for all queues
     * @param emergency_duration_seconds How long emergency mode should last
     * @return Number of queues that entered emergency mode
     */
    size_t handleEmergencyMode(DWORD emergency_duration_seconds);

    /**
     * @brief Exit emergency mode for all queues
     * @return Number of queues that exited emergency mode
     */
    size_t exitEmergencyMode();

    /**
     * @brief Preserve priority messages across all queues
     * @param count Number of messages to preserve per queue
     * @param duration_ms Preservation duration
     * @return Number of messages preserved
     */
    size_t preservePriorityMessages(size_t count, DWORD duration_ms);

    /**
     * @brief Perform emergency cleanup across all queues
     * @return Number of messages removed in emergency cleanup
     */
    size_t performEmergencyCleanup();

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Configure queue by ID
     * @param queue_id Queue ID to configure
     * @param config New configuration
     * @return true if queue was found and configured
     */
    bool configureQueue(uint32_t queue_id, const SmartQueueConfiguration& config);

    /**
     * @brief Get queue configuration by ID
     * @param queue_id Queue ID
     * @return Queue configuration or default config if not found
     */
    SmartQueueConfiguration getQueueConfiguration(uint32_t queue_id) const;

    /**
     * @brief Create default configuration for application
     * @param application_name Application name
     * @param queue_name Queue name
     * @return Default configuration
     */
    SmartQueueConfiguration createDefaultConfiguration(const std::string& application_name,
                                                       const std::string& queue_name);

    /**
     * @brief Create high-performance configuration for application
     * @param application_name Application name
     * @param queue_name Queue name
     * @return High-performance configuration
     */
    SmartQueueConfiguration createHighPerformanceConfiguration(const std::string& application_name,
                                                              const std::string& queue_name);

    // =================================================================================
    // STATISTICS AND REPORTING
    // =================================================================================

    /**
     * @brief Get comprehensive manager statistics
     * @return Detailed statistics as formatted string
     */
    std::string getComprehensiveStatistics() const;

    /**
     * @brief Get queue health status
     * @return Vector of queue health information
     */
    std::vector<std::pair<uint32_t, std::string>> getQueueHealthStatus() const;

    /**
     * @brief Get applications with queue information
     * @return Vector of application-queue information
     */
    std::vector<std::pair<std::string, size_t>> getApplicationQueueSummary() const;

    /**
     * @brief Export all queue statistics
     * @return Vector of all queue statistics
     */
    std::vector<SmartQueueStatistics> exportAllQueueStatistics() const;

    /**
     * @brief Reset all queue statistics
     */
    void resetAllStatistics();

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Clear all queues (use with caution)
     * @return Number of queues cleared
     */
    size_t clearAllQueues();

    /**
     * @brief Validate all queues
     * @return Vector of validation results (queue_id, is_valid)
     */
    std::vector<std::pair<uint32_t, bool>> validateAllQueues() const;

    /**
     * @brief Get manager status as formatted string
     * @return Status information
     */
    std::string getStatus() const;

    /**
     * @brief Check if manager is properly initialized
     * @return true if manager has valid state
     */
    bool isInitialized() const;

private:
    // Private helper methods
    uint32_t generateQueueId();
    void registerQueueWithApplication(uint32_t queue_id, const std::string& application_name);
    void unregisterQueueFromApplication(uint32_t queue_id, const std::string& application_name);
    std::string getApplicationNameFromQueue(uint32_t queue_id) const;
    void updateGlobalStatistics(const std::string& operation, size_t count);
    bool isQueueOverloaded(uint32_t queue_id) const;
    size_t getTotalApplicationQueueSize(const std::string& application_name) const;
    DWORD calculateCurrentThroughput() const;
};

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

/**
 * @brief Get global message queue manager (singleton pattern)
 * @return Reference to global MessageQueueManager
 */
MessageQueueManager& GetGlobalMessageQueueManager();

/**
 * @brief Create application queue in global manager
 * @param application_name Name of the application
 * @param queue_name Name for the queue
 * @param max_size Maximum queue size
 * @return Queue ID of the created queue
 */
uint32_t CreateGlobalApplicationQueue(const std::string& application_name,
                                     const std::string& queue_name,
                                     size_t max_size = 10000);

/**
 * @brief Distribute message to global queue manager
 * @param message Message to distribute
 * @param application_name Target application name
 * @param context Persistence context
 * @return true if message was queued successfully
 */
bool DistributeToGlobalQueue(const LogMessageData& message,
                            const std::string& application_name,
                            const PersistenceDecisionContext& context);

#endif // __ASFM_LOGGER_MESSAGE_QUEUE_MANAGER_HPP__