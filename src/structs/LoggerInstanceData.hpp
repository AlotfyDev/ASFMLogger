#ifndef __ASFM_LOGGER_INSTANCE_DATA_HPP__
#define __ASFM_LOGGER_INSTANCE_DATA_HPP__

/**
 * ASFMLogger Instance Management POD Structures
 *
 * Pure data structures for logger instance management following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include <cstdint>
#include <Windows.h>

// =====================================================================================
// LOGGER INSTANCE DATA STRUCTURE
// =====================================================================================

/**
 * @brief POD structure for logger instance information
 * Used for tracking multiple logger instances across applications and processes
 */
struct LoggerInstanceData {
    uint32_t instance_id;          ///< Unique instance identifier
    DWORD process_id;              ///< Windows process ID
    DWORD thread_id;               ///< Main thread ID (if applicable)
    char application_name[256];    ///< Application name
    char process_name[256];        ///< Process name
    char instance_name[256];       ///< Human-readable instance name
    char host_name[256];           ///< Machine hostname
    char user_name[256];           ///< User name
    DWORD creation_time;           ///< When instance was created (Unix timestamp)
    DWORD last_activity;           ///< Last activity timestamp
    uint64_t message_count;        ///< Total messages logged
    uint64_t error_count;          ///< Total errors logged
    char reserved[128];            ///< Future extensibility
};

/**
 * @brief POD structure for instance statistics
 * Used for monitoring and performance tracking
 */
struct InstanceStatistics {
    uint32_t instance_id;          ///< Instance identifier
    uint64_t total_messages;       ///< Total messages processed
    uint64_t messages_per_second;  ///< Current message rate
    uint64_t total_errors;         ///< Total errors encountered
    uint64_t error_rate;           ///< Current error rate
    DWORD uptime_seconds;          ///< Instance uptime in seconds
    size_t memory_usage;           ///< Memory usage in bytes
    char reserved[256];            ///< Future extensibility
};

/**
 * @brief POD structure for instance configuration
 * Used for instance-specific settings and policies
 */
struct InstanceConfiguration {
    uint32_t instance_id;          ///< Instance identifier
    bool enable_database_logging;  ///< Whether to log to database
    bool enable_shared_memory;     ///< Whether to use shared memory
    bool enable_file_logging;      ///< Whether to log to files
    bool enable_console_logging;   ///< Whether to log to console
    size_t max_queue_size;         ///< Maximum queue size before eviction
    DWORD flush_interval_ms;       ///< Queue flush interval in milliseconds
    char log_directory[512];       ///< Directory for log files
    char database_connection[512]; ///< Database connection string
    char shared_memory_name[256];  ///< Shared memory mapping name
    char reserved[256];            ///< Future extensibility
};

#endif // __ASFM_LOGGER_INSTANCE_DATA_HPP__