#ifndef __ASFM_LOGGER_PERSISTENCE_POLICY_HPP__
#define __ASFM_LOGGER_PERSISTENCE_POLICY_HPP__

/**
 * ASFMLogger Persistence Policy POD Structures
 *
 * Pure data structures for persistence policy configuration following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include "structs/LogDataStructures.hpp"
#include <cstdint>
#include <chrono>

// =====================================================================================
// PERSISTENCE POLICY DATA STRUCTURES
// =====================================================================================

/**
 * @brief POD structure for configurable persistence rules per application
 * Defines when and how messages should be persisted based on contextual factors
 */
struct PersistencePolicy {
    uint32_t policy_id;                        ///< Unique policy identifier
    char application_name[256];                ///< Application this policy applies to
    char policy_name[128];                     ///< Human-readable policy name
    char description[512];                     ///< Policy description

    // Core persistence rules
    MessageImportance min_importance;          ///< Minimum importance to persist
    bool persist_on_error;                     ///< Persist all messages when errors occur
    bool persist_on_critical;                  ///< Persist recent messages on critical errors
    size_t max_queue_size;                     ///< Maximum queue size before forced eviction
    DWORD max_age_seconds;                     ///< Maximum age before eviction

    // System condition thresholds
    DWORD high_load_threshold;                 ///< System load % threshold for high load mode
    DWORD high_error_rate_threshold;           ///< Error rate threshold for enhanced persistence
    DWORD emergency_mode_threshold;            ///< Error rate threshold for emergency mode

    // Adaptive behavior settings
    bool enable_adaptive_persistence;          ///< Whether to adapt based on system conditions
    DWORD adaptation_check_interval_seconds;   ///< How often to check for adaptation
    DWORD min_persistence_importance;          ///< Minimum importance even in low-load conditions
    DWORD max_persistence_importance;          ///< Maximum importance for performance protection

    // Component-specific rules
    char critical_components[10][128];         ///< Components that always get persisted
    size_t critical_component_count;           ///< Number of critical components
    char ignored_components[10][128];          ///< Components that never get persisted
    size_t ignored_component_count;            ///< Number of ignored components

    // Performance optimization
    bool enable_batch_persistence;             ///< Whether to batch persistence operations
    size_t batch_size;                         ///< Number of messages to batch together
    DWORD batch_timeout_ms;                    ///< Maximum time to wait before flushing batch

    // Emergency and recovery settings
    bool enable_emergency_persistence;         ///< Whether to enable emergency mode
    DWORD emergency_duration_seconds;          ///< How long emergency mode lasts
    MessageImportance emergency_importance_threshold;  ///< Importance threshold in emergency mode

    // Monitoring and statistics
    bool enable_persistence_statistics;        ///< Whether to track persistence statistics
    DWORD statistics_collection_interval;      ///< How often to collect statistics

    // Lifecycle management
    DWORD created_time;                        ///< When policy was created
    DWORD last_modified;                       ///< When policy was last modified
    char created_by[128];                      ///< Who created this policy
    char modified_by[128];                     ///< Who last modified this policy
    bool is_active;                            ///< Whether this policy is currently active

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for persistence decision context
 * Contains all runtime information needed for persistence decisions
 */
struct PersistenceDecisionContext {
    // System state
    DWORD current_system_load;                  ///< Current CPU/memory usage (0-100)
    DWORD current_error_rate;                  ///< Current errors per second
    DWORD current_message_rate;                ///< Current messages per second
    DWORD available_memory_mb;                 ///< Available system memory in MB
    DWORD available_disk_space_mb;             ///< Available disk space in MB

    // Application state
    char application_name[256];                ///< Application generating the message
    DWORD application_error_rate;              ///< Application-specific error rate
    DWORD application_message_rate;            ///< Application-specific message rate
    bool application_is_critical;              ///< Whether application is marked critical

    // Message context
    LogMessageType message_type;               ///< Type of message being evaluated
    char component[128];                       ///< Component that generated the message
    char function[128];                        ///< Function that generated the message
    MessageImportance resolved_importance;     ///< Importance resolved by importance framework

    // System conditions
    bool is_high_load;                         ///< Whether system is under high load
    bool is_emergency_mode;                    ///< Whether system is in emergency mode
    bool is_maintenance_window;                ///< Whether in scheduled maintenance window
    DWORD time_since_last_incident;            ///< Seconds since last critical incident

    // Performance context
    size_t current_queue_size;                 ///< Current size of persistence queue
    DWORD queue_processing_time_ms;            ///< Average queue processing time
    DWORD database_response_time_ms;           ///< Database response time
    bool database_is_available;                ///< Whether database is accessible

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for persistence decision result
 * Contains the decision and reasoning for persistence actions
 */
struct PersistenceDecisionResult {
    bool should_persist;                       ///< Whether message should be persisted
    char persistence_method[32];               ///< "DATABASE", "FILE", "SHARED_MEMORY", "NONE"
    char decision_reason[256];                 ///< Human-readable reason for decision
    DWORD decision_time_microseconds;          ///< Time taken to make decision
    DWORD estimated_persistence_time_ms;       ///< Estimated time to persist the message

    // Alternative actions
    bool should_queue_for_later;               ///< Whether to queue for later persistence
    bool should_drop_message;                  ///< Whether message should be dropped
    bool should_aggregate;                     ///< Whether message should be aggregated

    // Performance impact
    DWORD estimated_cpu_cost;                  ///< Estimated CPU cost of persistence
    DWORD estimated_memory_cost;               ///< Estimated memory cost
    DWORD estimated_disk_cost;                 ///< Estimated disk space cost

    // Debugging information
    MessageImportance effective_importance;    ///< Final importance used for decision
    DWORD system_load_at_decision;             ///< System load at decision time
    DWORD error_rate_at_decision;              ///< Error rate at decision time

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for persistence statistics
 * Tracks persistence performance and effectiveness metrics
 */
struct PersistenceStatistics {
    uint32_t policy_id;                        ///< Policy these statistics apply to
    DWORD collection_period_seconds;           ///< Period over which statistics were collected

    // Volume metrics
    uint64_t total_messages_evaluated;         ///< Total messages evaluated for persistence
    uint64_t messages_persisted;               ///< Messages actually persisted
    uint64_t messages_dropped;                 ///< Messages dropped due to policy
    uint64_t messages_queued;                  ///< Messages queued for later persistence

    // Performance metrics
    DWORD average_decision_time_microseconds;  ///< Average time to make persistence decision
    DWORD average_persistence_time_ms;         ///< Average time to persist a message
    DWORD max_persistence_time_ms;             ///< Maximum persistence time observed

    // System impact
    DWORD total_cpu_time_used_ms;              ///< Total CPU time used for persistence
    DWORD total_memory_allocated_mb;           ///< Memory allocated for persistence operations
    DWORD total_disk_space_used_mb;            ///< Disk space used for persistence

    // Error tracking
    uint64_t persistence_failures;             ///< Failed persistence attempts
    uint64_t database_timeouts;                ///< Database timeout errors
    uint64_t disk_space_errors;                ///< Disk space related errors

    // Effectiveness metrics
    double persistence_success_rate;           ///< Percentage of successful persistence operations
    double storage_efficiency_ratio;           ///< Ratio of persisted to evaluated messages
    DWORD policy_effectiveness_score;          ///< Overall policy effectiveness (0-100)

    // Collection metadata
    DWORD statistics_start_time;               ///< When statistics collection started
    DWORD statistics_end_time;                 ///< When statistics collection ended
    char collected_by[128];                    ///< Who collected these statistics

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for adaptive policy triggers
 * Defines conditions that trigger automatic policy adjustments
 */
struct AdaptivePolicyTrigger {
    uint32_t trigger_id;                       ///< Unique trigger identifier
    char trigger_name[128];                    ///< Human-readable trigger name
    char description[256];                     ///< Trigger description

    // Trigger conditions
    DWORD system_load_threshold;               ///< System load threshold to trigger
    DWORD error_rate_threshold;                ///< Error rate threshold to trigger
    DWORD message_rate_threshold;              ///< Message rate threshold to trigger
    DWORD queue_size_threshold;                ///< Queue size threshold to trigger

    // Trigger actions
    MessageImportance new_min_importance;      ///< New minimum importance when triggered
    bool enable_emergency_mode;                ///< Whether to enable emergency mode
    DWORD trigger_duration_seconds;            ///< How long trigger actions should last

    // Cooldown settings
    DWORD cooldown_period_seconds;             ///< Minimum time between trigger activations
    DWORD last_triggered_time;                 ///< When trigger was last activated

    // Trigger state
    bool is_enabled;                           ///< Whether trigger is currently enabled
    DWORD activation_count;                    ///< Number of times trigger has activated

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

#endif // __ASFM_LOGGER_PERSISTENCE_POLICY_HPP__