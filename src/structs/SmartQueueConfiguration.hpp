#ifndef __ASFM_LOGGER_SMART_QUEUE_CONFIG_HPP__
#define __ASFM_LOGGER_SMART_QUEUE_CONFIG_HPP__

/**
 * ASFMLogger Smart Queue POD Structures
 *
 * Pure data structures for smart queue configuration following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include "LogDataStructures.hpp"
#include <cstdint>

// =====================================================================================
// SMART QUEUE DATA STRUCTURES
// =====================================================================================

/**
 * @brief POD structure for smart queue configuration
 * Defines behavior and limits for intelligent message queuing
 */
struct SmartQueueConfiguration {
    uint32_t queue_id;                         ///< Unique queue identifier
    char queue_name[128];                      ///< Human-readable queue name
    char application_name[256];                ///< Application this queue serves

    // Size and capacity limits
    size_t max_total_size;                     ///< Maximum total queue size
    size_t max_priority_size;                  ///< Maximum size for priority messages
    size_t max_normal_size;                    ///< Maximum size for normal messages
    size_t max_bulk_size;                      ///< Maximum size for bulk operations

    // Importance-based sizing
    size_t critical_messages_reserved;         ///< Reserved space for critical messages
    size_t high_messages_reserved;             ///< Reserved space for high importance messages
    size_t medium_messages_reserved;           ///< Reserved space for medium importance messages
    size_t low_messages_reserved;              ///< Reserved space for low importance messages

    // Eviction policies
    bool enable_importance_based_eviction;     ///< Whether to use importance for eviction
    bool enable_time_based_eviction;           ///< Whether to use age for eviction
    bool enable_size_based_eviction;           ///< Whether to use size for eviction
    DWORD max_message_age_seconds;             ///< Maximum age before eviction

    // Performance settings
    bool enable_batch_processing;              ///< Whether to enable batch processing
    size_t batch_size;                         ///< Number of messages per batch
    DWORD batch_timeout_ms;                    ///< Maximum time to wait for batch

    // Priority preservation
    bool enable_priority_preservation;         ///< Whether to preserve high-priority messages
    size_t priority_preservation_count;        ///< Number of priority messages to preserve
    DWORD priority_preservation_duration_ms;   ///< How long to preserve priority messages

    // Monitoring and statistics
    bool enable_detailed_statistics;           ///< Whether to collect detailed statistics
    DWORD statistics_collection_interval_ms;   ///< Statistics collection frequency

    // Threading and synchronization
    bool enable_lock_free_operations;           ///< Whether to use lock-free operations
    DWORD spin_lock_timeout_ms;                ///< Timeout for spin lock attempts

    // Lifecycle management
    DWORD created_time;                        ///< When queue was created
    DWORD last_modified;                       ///< When configuration was last modified
    char created_by[128];                      ///< Who created this queue
    bool is_active;                            ///< Whether this queue is active

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for queue entry metadata
 * Additional information stored with each queued message
 */
struct QueueEntryMetadata {
    uint32_t message_id;                       ///< Original message ID
    DWORD queued_time;                         ///< When message was queued
    DWORD priority_score;                      ///< Calculated priority score
    size_t message_size;                       ///< Size of the message in bytes
    bool is_priority_preserved;                ///< Whether message is priority preserved
    DWORD preservation_expiry;                 ///< When preservation expires
    char reserved[128];                        ///< Future extensibility
};

/**
 * @brief POD structure for queue statistics
 * Tracks queue performance and behavior metrics
 */
struct SmartQueueStatistics {
    uint32_t queue_id;                         ///< Queue identifier
    DWORD collection_period_seconds;           ///< Period over which statistics were collected

    // Volume metrics
    uint64_t total_messages_queued;            ///< Total messages added to queue
    uint64_t total_messages_dequeued;          ///< Total messages removed from queue
    uint64_t total_messages_evicted;           ///< Total messages evicted due to policy
    uint64_t current_queue_size;               ///< Current number of messages in queue

    // Performance metrics
    DWORD average_queue_latency_ms;            ///< Average time messages spend in queue
    DWORD max_queue_latency_ms;                ///< Maximum observed queue latency
    DWORD average_processing_time_ms;          ///< Average time to process queued messages
    DWORD queue_throughput_per_second;         ///< Messages processed per second

    // Importance distribution
    uint64_t critical_messages_queued;         ///< Critical messages in queue
    uint64_t high_messages_queued;             ///< High importance messages in queue
    uint64_t medium_messages_queued;           ///< Medium importance messages in queue
    uint64_t low_messages_queued;              ///< Low importance messages in queue

    // Eviction statistics
    uint64_t importance_evictions;             ///< Messages evicted due to importance
    uint64_t age_evictions;                    ///< Messages evicted due to age
    uint64_t size_evictions;                   ///< Messages evicted due to size
    DWORD last_eviction_time;                  ///< When last eviction occurred

    // Memory usage
    size_t memory_used_bytes;                  ///< Current memory usage
    size_t peak_memory_usage_bytes;            ///< Peak memory usage observed
    DWORD memory_pressure_events;              ///< Number of memory pressure events

    // Error tracking
    uint64_t queue_full_errors;                ///< Times queue was full when trying to add
    uint64_t eviction_failures;                ///< Failed eviction attempts
    uint64_t corruption_events;                ///< Data corruption events detected

    // Collection metadata
    DWORD statistics_start_time;               ///< When statistics collection started
    DWORD statistics_end_time;                 ///< When statistics collection ended
    char collected_by[128];                    ///< Who collected these statistics

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for queue eviction decision
 * Contains information about why and how a message was evicted
 */
struct QueueEvictionDecision {
    uint32_t message_id;                       ///< ID of evicted message
    DWORD eviction_time;                       ///< When eviction occurred
    char eviction_reason[128];                 ///< Reason for eviction
    char eviction_method[64];                  ///< Method used for eviction ("IMPORTANCE", "AGE", "SIZE")
    DWORD queue_size_before;                   ///< Queue size before eviction
    DWORD queue_size_after;                    ///< Queue size after eviction
    DWORD memory_freed_bytes;                  ///< Memory freed by eviction
    char reserved[256];                        ///< Future extensibility
};

/**
 * @brief POD structure for queue batch operation
 * Represents a batch of messages for efficient processing
 */
struct QueueBatch {
    uint32_t batch_id;                         ///< Unique batch identifier
    DWORD created_time;                        ///< When batch was created
    size_t message_count;                      ///< Number of messages in batch
    size_t total_size_bytes;                   ///< Total size of all messages
    DWORD max_age_in_batch_ms;                 ///< Age of oldest message in batch
    bool is_priority_batch;                    ///< Whether this is a priority batch
    char batch_type[32];                       ///< Type of batch ("PERSISTENCE", "PROCESSING", "EVICTION")
    char reserved[256];                        ///< Future extensibility
};

#endif // __ASFM_LOGGER_SMART_QUEUE_CONFIG_HPP__