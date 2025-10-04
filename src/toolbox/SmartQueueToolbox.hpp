#ifndef __ASFM_LOGGER_SMART_QUEUE_TOOLBOX_HPP__
#define __ASFM_LOGGER_SMART_QUEUE_TOOLBOX_HPP__

/**
 * ASFMLogger Smart Queue Toolbox
 *
 * Static methods for smart queue operations following toolbox architecture.
 * Pure functions for intelligent message buffering and prioritization.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/SmartQueueConfiguration.hpp"
#include "structs/PersistencePolicy.hpp"
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>

// Forward declarations
struct LogMessageData;

class SmartQueueToolbox {
private:
    // Only static variables allowed in toolbox
    static std::unordered_map<uint32_t, SmartQueueConfiguration> queue_configurations_;
    static std::unordered_map<uint32_t, SmartQueueStatistics> queue_statistics_;

public:
    // =================================================================================
    // QUEUE ENTRY MANAGEMENT
    // =================================================================================

    /**
     * @brief Create queue entry metadata for a message
     * @param message Message to create entry for
     * @param priority_score Calculated priority score
     * @return Queue entry metadata
     */
    static QueueEntryMetadata CreateQueueEntry(const LogMessageData& message, DWORD priority_score);

    /**
     * @brief Update queue entry metadata
     * @param entry Entry to update
     * @param new_priority_score New priority score (optional)
     * @param preserve_priority Whether to extend priority preservation
     * @return Updated entry metadata
     */
    static QueueEntryMetadata UpdateQueueEntry(QueueEntryMetadata& entry,
                                              DWORD new_priority_score = 0,
                                              bool preserve_priority = false);

    /**
     * @brief Check if queue entry is expired
     * @param entry Entry to check
     * @param current_time Current timestamp
     * @return true if entry has expired
     */
    static bool IsQueueEntryExpired(const QueueEntryMetadata& entry, DWORD current_time);

    /**
     * @brief Check if queue entry priority preservation has expired
     * @param entry Entry to check
     * @param current_time Current timestamp
     * @return true if preservation has expired
     */
    static bool IsPriorityPreservationExpired(const QueueEntryMetadata& entry, DWORD current_time);

    // =================================================================================
    // PRIORITY CALCULATION
    // =================================================================================

    /**
     * @brief Calculate priority score for a message
     * @param message Message to score
     * @param config Queue configuration
     * @param context Additional context for scoring
     * @return Priority score (higher = more important)
     */
    static DWORD CalculatePriorityScore(const LogMessageData& message,
                                       const SmartQueueConfiguration& config,
                                       const PersistenceDecisionContext& context);

    /**
     * @brief Calculate base priority from message type and importance
     * @param message_type Type of the message
     * @param importance Importance level
     * @return Base priority score
     */
    static DWORD CalculateBasePriority(LogMessageType message_type, MessageImportance importance);

    /**
     * @brief Calculate contextual priority bonus
     * @param message Message to evaluate
     * @param context System context
     * @return Bonus score based on context
     */
    static DWORD CalculateContextualBonus(const LogMessageData& message,
                                         const PersistenceDecisionContext& context);

    /**
     * @brief Calculate time-based priority (newer messages get slight bonus)
     * @param message_age_seconds Age of message in seconds
     * @return Time-based priority adjustment
     */
    static DWORD CalculateTimePriority(DWORD message_age_seconds);

    // =================================================================================
    // QUEUE SPACE MANAGEMENT
    // =================================================================================

    /**
     * @brief Check if message can fit in queue
     * @param message Message to check
     * @param current_queue_size Current queue size
     * @param config Queue configuration
     * @return true if message can fit
     */
    static bool CanMessageFit(const LogMessageData& message,
                             size_t current_queue_size,
                             const SmartQueueConfiguration& config);

    /**
     * @brief Calculate space needed for a message
     * @param message Message to measure
     * @return Space needed in bytes
     */
    static size_t CalculateMessageSpace(const LogMessageData& message);

    /**
     * @brief Check if queue has reserved space for importance level
     * @param importance Importance level to check
     * @param current_usage Current space usage by importance
     * @param config Queue configuration
     * @return true if space is available
     */
    static bool HasReservedSpace(MessageImportance importance,
                                const std::unordered_map<MessageImportance, size_t>& current_usage,
                                const SmartQueueConfiguration& config);

    /**
     * @brief Update space usage tracking
     * @param current_usage Current usage map to update
     * @param importance Importance level
     * @param space_used Space used or freed
     * @param is_adding Whether space is being added or removed
     */
    static void UpdateSpaceUsage(std::unordered_map<MessageImportance, size_t>& current_usage,
                                MessageImportance importance,
                                size_t space_used,
                                bool is_adding);

    // =================================================================================
    // INTELLIGENT EVICTION STRATEGIES
    // =================================================================================

    /**
     * @brief Find best message to evict based on importance
     * @param queue Queue to analyze (message, metadata pairs)
     * @param space_needed Space needed to free
     * @param config Queue configuration
     * @return Vector of messages to evict
     */
    static std::vector<std::pair<LogMessageData, QueueEntryMetadata>> FindMessagesToEvict(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        size_t space_needed,
        const SmartQueueConfiguration& config);

    /**
     * @brief Find oldest messages for age-based eviction
     * @param queue Queue to analyze
     * @param count Number of messages to find
     * @param current_time Current timestamp
     * @return Vector of oldest messages
     */
    static std::vector<std::pair<LogMessageData, QueueEntryMetadata>> FindOldestMessages(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        size_t count,
        DWORD current_time);

    /**
     * @brief Find lowest priority messages for importance-based eviction
     * @param queue Queue to analyze
     * @param count Number of messages to find
     * @return Vector of lowest priority messages
     */
    static std::vector<std::pair<LogMessageData, QueueEntryMetadata>> FindLowestPriorityMessages(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        size_t count);

    /**
     * @brief Find messages that are not priority preserved
     * @param queue Queue to analyze
     * @param count Number of messages to find
     * @param current_time Current timestamp
     * @return Vector of non-preserved messages
     */
    static std::vector<std::pair<LogMessageData, QueueEntryMetadata>> FindNonPreservedMessages(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        size_t count,
        DWORD current_time);

    /**
     * @brief Execute eviction decision
     * @param messages_to_evict Messages to remove from queue
     * @param reason Reason for eviction
     * @return Vector of eviction decisions
     */
    static std::vector<QueueEvictionDecision> ExecuteEviction(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& messages_to_evict,
        const std::string& reason);

    // =================================================================================
    // PRIORITY PRESERVATION
    // =================================================================================

    /**
     * @brief Mark messages for priority preservation
     * @param messages Messages to preserve
     * @param preservation_duration_ms How long to preserve
     * @return Updated metadata with preservation settings
     */
    static std::vector<QueueEntryMetadata> MarkForPriorityPreservation(
        const std::vector<LogMessageData>& messages,
        DWORD preservation_duration_ms);

    /**
     * @brief Extend priority preservation for existing entries
     * @param entries Entries to extend preservation for
     * @param additional_duration_ms Additional preservation time
     * @return Updated entries with extended preservation
     */
    static std::vector<QueueEntryMetadata> ExtendPriorityPreservation(
        const std::vector<QueueEntryMetadata>& entries,
        DWORD additional_duration_ms);

    /**
     * @brief Find messages that should be preserved during eviction
     * @param queue Queue to analyze
     * @param eviction_count Number of messages being evicted
     * @param current_time Current timestamp
     * @return Messages that should be preserved
     */
    static std::vector<std::pair<LogMessageData, QueueEntryMetadata>> FindPreservedMessages(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        size_t eviction_count,
        DWORD current_time);

    /**
     * @brief Check if message should be preserved from eviction
     * @param entry Queue entry to check
     * @param current_time Current timestamp
     * @param config Queue configuration
     * @return true if message should be preserved
     */
    static bool ShouldPreserveMessage(const QueueEntryMetadata& entry,
                                     DWORD current_time,
                                     const SmartQueueConfiguration& config);

    // =================================================================================
    // BATCH OPERATIONS
    // =================================================================================

    /**
     * @brief Create optimal batch for processing
     * @param queue Queue to create batch from
     * @param config Queue configuration
     * @param max_batch_size Maximum batch size
     * @return Optimal batch for current conditions
     */
    static QueueBatch CreateOptimalBatch(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        const SmartQueueConfiguration& config,
        size_t max_batch_size);

    /**
     * @brief Create priority batch for urgent processing
     * @param queue Queue to create batch from
     * @param config Queue configuration
     * @return Priority batch with most important messages
     */
    static QueueBatch CreatePriorityBatch(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        const SmartQueueConfiguration& config);

    /**
     * @brief Create age-based batch for cleanup
     * @param queue Queue to create batch from
     * @param config Queue configuration
     * @param current_time Current timestamp
     * @return Batch of oldest messages
     */
    static QueueBatch CreateAgeBasedBatch(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
        const SmartQueueConfiguration& config,
        DWORD current_time);

    /**
     * @brief Validate batch integrity
     * @param batch Batch to validate
     * @return true if batch is valid
     */
    static bool ValidateBatch(const QueueBatch& batch);

    // =================================================================================
    // QUEUE ORGANIZATION AND SORTING
    // =================================================================================

    /**
     * @brief Sort queue by priority (highest first)
     * @param queue Queue to sort (modified in place)
     */
    static void SortByPriority(std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue);

    /**
     * @brief Sort queue by age (oldest first)
     * @param queue Queue to sort (modified in place)
     * @param current_time Current timestamp
     */
    static void SortByAge(std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
                         DWORD current_time);

    /**
     * @brief Sort queue by size (largest first)
     * @param queue Queue to sort (modified in place)
     */
    static void SortBySize(std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue);

    /**
     * @brief Group queue by importance level
     * @param queue Queue to group
     * @return Map of importance level to message vectors
     */
    static std::unordered_map<MessageImportance, std::vector<LogMessageData>> GroupByImportance(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue);

    /**
     * @brief Group queue by message type
     * @param queue Queue to group
     * @return Map of message type to message vectors
     */
    static std::unordered_map<LogMessageType, std::vector<LogMessageData>> GroupByMessageType(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue);

    // =================================================================================
    // STATISTICS AND MONITORING
    // =================================================================================

    /**
     * @brief Update queue statistics
     * @param queue_id Queue identifier
     * @param operation Operation performed ("QUEUE", "DEQUEUE", "EVICT")
     * @param message_count Number of messages affected
     * @param bytes_affected Number of bytes affected
     */
    static void UpdateQueueStatistics(uint32_t queue_id,
                                     const std::string& operation,
                                     size_t message_count,
                                     size_t bytes_affected);

    /**
     * @brief Get queue statistics
     * @param queue_id Queue identifier
     * @return Queue statistics or empty stats if not found
     */
    static SmartQueueStatistics GetQueueStatistics(uint32_t queue_id);

    /**
     * @brief Reset queue statistics
     * @param queue_id Queue identifier
     */
    static void ResetQueueStatistics(uint32_t queue_id);

    /**
     * @brief Analyze queue performance
     * @param queue_id Queue identifier
     * @return Performance analysis as formatted string
     */
    static std::string AnalyzeQueuePerformance(uint32_t queue_id);

    /**
     * @brief Calculate queue efficiency metrics
     * @param queue_id Queue identifier
     * @return Vector of efficiency metrics
     */
    static std::vector<std::pair<std::string, double>> CalculateEfficiencyMetrics(uint32_t queue_id);

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Set queue configuration
     * @param queue_id Queue identifier
     * @param config Queue configuration
     * @return true if configuration was set successfully
     */
    static bool SetQueueConfiguration(uint32_t queue_id, const SmartQueueConfiguration& config);

    /**
     * @brief Get queue configuration
     * @param queue_id Queue identifier
     * @return Queue configuration or default config if not found
     */
    static SmartQueueConfiguration GetQueueConfiguration(uint32_t queue_id);

    /**
     * @brief Create default queue configuration
     * @param queue_name Name for the queue
     * @param application_name Application name
     * @return Default configuration
     */
    static SmartQueueConfiguration CreateDefaultConfiguration(const std::string& queue_name,
                                                            const std::string& application_name);

    /**
     * @brief Create high-performance queue configuration
     * @param queue_name Name for the queue
     * @param application_name Application name
     * @return High-performance configuration
     */
    static SmartQueueConfiguration CreateHighPerformanceConfiguration(const std::string& queue_name,
                                                                     const std::string& application_name);

    /**
     * @brief Validate queue configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    static bool ValidateQueueConfiguration(const SmartQueueConfiguration& config);

    // =================================================================================
    // MEMORY MANAGEMENT
    // =================================================================================

    /**
     * @brief Calculate memory pressure level (0-100)
     * @param current_usage Current memory usage
     * @param max_capacity Maximum capacity
     * @return Memory pressure percentage
     */
    static DWORD CalculateMemoryPressure(size_t current_usage, size_t max_capacity);

    /**
     * @brief Determine if memory pressure requires immediate action
     * @param current_usage Current memory usage
     * @param max_capacity Maximum capacity
     * @param pressure_threshold Threshold for action (0-100)
     * @return true if immediate action needed
     */
    static bool IsMemoryPressureCritical(size_t current_usage,
                                        size_t max_capacity,
                                        DWORD pressure_threshold = 90);

    /**
     * @brief Calculate optimal memory allocation for queue
     * @param message_rate Expected message rate
     * @param average_message_size Average message size
     * @param retention_time_seconds How long to retain messages
     * @return Optimal memory allocation in bytes
     */
    static size_t CalculateOptimalMemoryAllocation(DWORD message_rate,
                                                  size_t average_message_size,
                                                  DWORD retention_time_seconds);

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Convert eviction decision to string
     * @param decision Decision to format
     * @return Human-readable decision description
     */
    static std::string EvictionDecisionToString(const QueueEvictionDecision& decision);

    /**
     * @brief Convert queue configuration to string
     * @param config Configuration to format
     * @return Human-readable configuration description
     */
    static std::string QueueConfigurationToString(const SmartQueueConfiguration& config);

    /**
     * @brief Convert queue statistics to string
     * @param stats Statistics to format
     * @return Human-readable statistics description
     */
    static std::string QueueStatisticsToString(const SmartQueueStatistics& stats);

    /**
     * @brief Get current timestamp for queue operations
     * @return Current timestamp
     */
    static DWORD GetCurrentTimestamp();

public:
    // =================================================================================
    // PUBLIC HELPER METHODS
    // =================================================================================

    /**
     * @brief Generate unique queue identifier
     * @return Unique queue ID
     */
    static uint32_t GenerateQueueId();

    /**
     * @brief Generate unique batch identifier
     * @return Unique batch ID
     */
    static uint32_t GenerateBatchId();

    /**
     * @brief Calculate age of a queue entry
     * @param entry Queue entry to check
     * @param current_time Current timestamp
     * @return Age in seconds
     */
    static DWORD CalculateMessageAge(const QueueEntryMetadata& entry, DWORD current_time);

    /**
     * @brief Compare two queue entries by priority
     * @param a First entry
     * @param b Second entry
     * @return true if a has higher priority than b
     */
    static bool ComparePriority(const std::pair<LogMessageData, QueueEntryMetadata>& a,
                               const std::pair<LogMessageData, QueueEntryMetadata>& b);

    /**
     * @brief Compare two queue entries by age
     * @param a First entry
     * @param b Second entry
     * @param current_time Current timestamp
     * @return true if a is older than b
     */
    static bool CompareAge(const std::pair<LogMessageData, QueueEntryMetadata>& a,
                          const std::pair<LogMessageData, QueueEntryMetadata>& b,
                          DWORD current_time);

    /**
     * @brief Compare two messages for equality
     * @param a First message
     * @param b Second message
     * @return true if messages are identical
     */
    static bool CompareMessages(const LogMessageData& a, const LogMessageData& b);

    /**
     * @brief Convert deque to vector for ToolBox operations
     * @param deque_container Source deque container
     * @return Vector container with same contents
     */
    static std::vector<std::pair<LogMessageData, QueueEntryMetadata>> DequeToVector(
        const std::deque<std::pair<LogMessageData, QueueEntryMetadata>>& deque_container);

    /**
     * @brief Convert vector to deque for stateful operations
     * @param vector_container Source vector container
     * @return Deque container with same contents
     */
    static std::deque<std::pair<LogMessageData, QueueEntryMetadata>> VectorToDeque(
        const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& vector_container);

    /**
     * @brief Estimate optimal batch size
     * @param candidates Messages to batch
     * @param max_size Maximum batch size
     * @return Estimated optimal batch size
     */
    static size_t EstimateBatchSize(const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& candidates,
                                   size_t max_size);

private:
    // Private helper methods
    static void InitializeDefaultConfigurations();
    static bool IsInitialized();
};

#endif // __ASFM_LOGGER_SMART_QUEUE_TOOLBOX_HPP__