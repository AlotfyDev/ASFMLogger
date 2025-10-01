#ifndef __ASFM_LOGGER_MONITORING_DATA_HPP__
#define __ASFM_LOGGER_MONITORING_DATA_HPP__

/**
 * ASFMLogger Monitoring and Adaptation POD Structures
 *
 * Pure data structures for monitoring and adaptation following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include "structs/LogDataStructures.hpp"
#include <cstdint>

// =====================================================================================
// PERFORMANCE MONITORING DATA STRUCTURES
// =====================================================================================

/**
 * @brief POD structure for system performance metrics
 * Tracks real-time system performance indicators
 */
struct SystemPerformanceMetrics {
    uint32_t collection_id;                    ///< Unique collection identifier
    DWORD collection_time;                     ///< When metrics were collected

    // CPU metrics
    DWORD cpu_usage_percent;                   ///< Current CPU usage (0-100)
    DWORD cpu_kernel_percent;                  ///< CPU time in kernel mode
    DWORD cpu_user_percent;                    ///< CPU time in user mode
    DWORD cpu_queue_length;                    ///< CPU queue length

    // Memory metrics
    size_t total_memory_mb;                    ///< Total system memory in MB
    size_t available_memory_mb;                ///< Available memory in MB
    size_t used_memory_mb;                     ///< Used memory in MB
    size_t memory_usage_percent;               ///< Memory usage percentage

    // Disk metrics
    size_t total_disk_space_mb;                ///< Total disk space in MB
    size_t available_disk_space_mb;            ///< Available disk space in MB
    size_t disk_usage_percent;                 ///< Disk usage percentage
    DWORD disk_read_rate_kb_per_sec;           ///< Disk read rate
    DWORD disk_write_rate_kb_per_sec;          ///< Disk write rate

    // Network metrics
    DWORD network_bytes_in_per_sec;            ///< Network bytes received per second
    DWORD network_bytes_out_per_sec;           ///< Network bytes sent per second
    DWORD network_connections_active;          ///< Active network connections

    // Process-specific metrics
    size_t process_memory_usage_mb;            ///< Memory used by logging process
    DWORD process_cpu_usage_percent;           ///< CPU used by logging process
    DWORD process_thread_count;                ///< Number of threads in logging process
    DWORD process_handle_count;                ///< Number of handles in logging process

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for logging system health metrics
 * Tracks the health and performance of the logging system itself
 */
struct LoggingSystemHealth {
    uint32_t health_check_id;                  ///< Unique health check identifier
    DWORD check_time;                          ///< When health check was performed

    // Queue health
    size_t total_queue_count;                  ///< Total number of queues
    size_t healthy_queues;                     ///< Number of healthy queues
    size_t degraded_queues;                    ///< Number of degraded queues
    size_t critical_queues;                    ///< Number of critical queues

    // Database health
    bool database_connected;                   ///< Whether database is connected
    DWORD database_response_time_ms;           ///< Database response time
    size_t database_connection_count;          ///< Number of database connections
    size_t database_error_count;               ///< Database errors in check period

    // Shared memory health
    bool shared_memory_available;              ///< Whether shared memory is available
    size_t shared_memory_regions;              ///< Number of shared memory regions
    size_t shared_memory_total_size;           ///< Total shared memory size
    size_t shared_memory_used_size;            ///< Used shared memory size

    // Performance indicators
    DWORD average_message_processing_time_ms;  ///< Average time to process a message
    DWORD message_throughput_per_second;       ///< Messages processed per second
    DWORD error_rate_per_second;               ///< Errors per second
    DWORD queue_overflow_events;               ///< Queue overflow events

    // System stress indicators
    bool is_under_stress;                      ///< Whether system is under stress
    DWORD stress_level;                        ///< Current stress level (0-100)
    char primary_stressor[128];                ///< Primary cause of stress
    DWORD time_under_stress_seconds;           ///< How long system has been under stress

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for adaptive behavior triggers
 * Defines conditions that trigger automatic system adaptation
 */
struct AdaptiveBehaviorTrigger {
    uint32_t trigger_id;                       ///< Unique trigger identifier
    char trigger_name[128];                    ///< Human-readable trigger name
    char trigger_type[64];                     ///< Type of trigger ("PERFORMANCE", "ERROR", "LOAD")

    // Trigger conditions
    DWORD cpu_threshold_percent;               ///< CPU usage threshold
    DWORD memory_threshold_percent;            ///< Memory usage threshold
    DWORD error_rate_threshold;                ///< Error rate threshold
    DWORD queue_size_threshold;                ///< Queue size threshold

    // Trigger actions
    char adaptation_action[128];               ///< Action to take when triggered
    DWORD new_importance_threshold;            ///< New importance threshold
    bool enable_emergency_mode;                ///< Whether to enable emergency mode
    DWORD adaptation_duration_seconds;         ///< How long adaptation should last

    // Cooldown and state
    DWORD cooldown_period_seconds;             ///< Minimum time between triggers
    DWORD last_triggered_time;                 ///< When trigger was last activated
    DWORD trigger_count;                       ///< Number of times triggered
    bool is_enabled;                           ///< Whether trigger is enabled

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for system adaptation record
 * Records when and how the system adapted to conditions
 */
struct SystemAdaptationRecord {
    uint32_t adaptation_id;                    ///< Unique adaptation identifier
    char adaptation_type[128];                 ///< Type of adaptation performed
    char trigger_reason[256];                  ///< Why adaptation was triggered

    // Before/after values
    DWORD cpu_usage_before;                    ///< CPU usage before adaptation
    DWORD cpu_usage_after;                     ///< CPU usage after adaptation
    DWORD memory_usage_before;                 ///< Memory usage before adaptation
    DWORD memory_usage_after;                  ///< Memory usage after adaptation
    MessageImportance importance_threshold_before;  ///< Importance threshold before
    MessageImportance importance_threshold_after;   ///< Importance threshold after

    // Adaptation details
    DWORD adaptation_start_time;               ///< When adaptation started
    DWORD adaptation_end_time;                 ///< When adaptation ended
    DWORD adaptation_duration_seconds;         ///< How long adaptation lasted
    bool adaptation_successful;                ///< Whether adaptation achieved goals

    // Impact assessment
    DWORD messages_processed_during_adaptation;  ///< Messages processed during adaptation
    DWORD errors_during_adaptation;            ///< Errors during adaptation
    char adaptation_effectiveness[64];         ///< Effectiveness rating

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for comprehensive logging statistics
 * Aggregated statistics across all logging components
 */
struct ComprehensiveLoggingStatistics {
    uint32_t statistics_id;                    ///< Unique statistics identifier
    char application_name[256];                ///< Application these statistics apply to
    DWORD collection_period_seconds;           ///< Period over which statistics were collected

    // Message volume statistics
    uint64_t total_messages_logged;            ///< Total messages logged
    uint64_t messages_by_type[6];              ///< Messages by type (TRACE, DEBUG, INFO, WARN, ERR, CRITICAL)
    uint64_t messages_by_importance[4];        ///< Messages by importance level
    DWORD average_messages_per_second;         ///< Average message rate

    // Performance statistics
    DWORD average_message_processing_time_ms;  ///< Average processing time per message
    DWORD max_message_processing_time_ms;      ///< Maximum processing time observed
    DWORD queue_wait_time_average_ms;          ///< Average time messages wait in queue
    DWORD database_insertion_time_average_ms;  ///< Average database insertion time

    // Error statistics
    uint64_t total_errors;                     ///< Total errors encountered
    uint64_t errors_by_type[6];                ///< Errors by message type
    DWORD error_rate_per_second;               ///< Current error rate
    char most_common_error[256];               ///< Most common error message

    // Resource utilization
    size_t peak_memory_usage_mb;               ///< Peak memory usage
    size_t average_memory_usage_mb;            ///< Average memory usage
    size_t disk_space_used_mb;                 ///< Disk space used for logging
    DWORD cpu_time_used_seconds;               ///< CPU time used for logging

    // Queue statistics
    size_t total_queues;                       ///< Total number of queues
    size_t active_queues;                      ///< Number of active queues
    uint64_t total_queue_overflows;            ///< Total queue overflow events
    DWORD average_queue_size;                  ///< Average queue size across all queues

    // Database statistics
    uint64_t database_operations_total;        ///< Total database operations
    uint64_t database_operations_successful;   ///< Successful database operations
    uint64_t database_operations_failed;       ///< Failed database operations
    DWORD database_connection_count;           ///< Current database connections

    // Shared memory statistics
    size_t shared_memory_regions_active;       ///< Active shared memory regions
    size_t shared_memory_total_size_mb;        ///< Total shared memory size
    size_t shared_memory_used_size_mb;         ///< Used shared memory size
    uint64_t shared_memory_messages_sent;      ///< Messages sent via shared memory

    // System health indicators
    bool system_healthy;                       ///< Overall system health
    DWORD system_health_score;                 ///< Health score (0-100)
    char health_issues[512];                   ///< Description of health issues

    // Collection metadata
    DWORD statistics_start_time;               ///< When statistics collection started
    DWORD statistics_end_time;                 ///< When statistics collection ended
    char collected_by[128];                    ///< Who collected these statistics

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for performance trend analysis
 * Tracks performance trends over time for predictive adaptation
 */
struct PerformanceTrendAnalysis {
    uint32_t analysis_id;                      ///< Unique analysis identifier
    char analysis_period[64];                  ///< Period analyzed ("1H", "24H", "7D")
    DWORD analysis_start_time;                 ///< When analysis period started
    DWORD analysis_end_time;                   ///< When analysis period ended

    // Trend indicators
    bool cpu_usage_trending_up;                ///< Whether CPU usage is trending up
    bool memory_usage_trending_up;              ///< Whether memory usage is trending up
    bool error_rate_trending_up;               ///< Whether error rate is trending up
    bool message_rate_trending_up;             ///< Whether message rate is trending up

    // Trend rates
    double cpu_usage_change_rate;              ///< CPU usage change rate per hour
    double memory_usage_change_rate;           ///< Memory usage change rate per hour
    double error_rate_change_rate;             ///< Error rate change rate per hour
    double message_rate_change_rate;           ///< Message rate change rate per hour

    // Predictive indicators
    DWORD predicted_cpu_usage_1h;              ///< Predicted CPU usage in 1 hour
    DWORD predicted_memory_usage_1h;           ///< Predicted memory usage in 1 hour
    DWORD predicted_error_rate_1h;             ///< Predicted error rate in 1 hour
    char performance_prediction[128];          ///< Overall performance prediction

    // Anomaly detection
    bool anomalies_detected;                   ///< Whether anomalies were detected
    size_t anomaly_count;                      ///< Number of anomalies detected
    char primary_anomaly[256];                 ///< Description of primary anomaly

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for monitoring alert configuration
 * Defines when and how to generate monitoring alerts
 */
struct MonitoringAlertConfiguration {
    uint32_t alert_config_id;                  ///< Unique alert configuration identifier
    char alert_name[128];                      ///< Human-readable alert name
    char alert_type[64];                       ///< Type of alert ("PERFORMANCE", "ERROR", "HEALTH")

    // Alert thresholds
    DWORD cpu_threshold_percent;               ///< CPU usage threshold for alert
    DWORD memory_threshold_percent;            ///< Memory usage threshold for alert
    DWORD error_rate_threshold;                ///< Error rate threshold for alert
    DWORD queue_size_threshold;                ///< Queue size threshold for alert

    // Alert behavior
    bool alert_enabled;                        ///< Whether alert is enabled
    DWORD alert_cooldown_seconds;              ///< Minimum time between alerts
    DWORD last_alert_time;                     ///< When last alert was sent
    size_t alert_count;                        ///< Number of alerts sent

    // Alert targets
    char webhook_url[512];                     ///< Webhook URL for alert delivery
    char email_recipients[512];                ///< Email addresses for alerts
    char slack_channel[256];                   ///< Slack channel for alerts

    // Alert content
    char alert_title_template[256];            ///< Template for alert title
    char alert_message_template[512];          ///< Template for alert message
    bool include_system_metrics;               ///< Whether to include system metrics in alert

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

#endif // __ASFM_LOGGER_MONITORING_DATA_HPP__