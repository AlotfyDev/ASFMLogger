#ifndef __ASFM_LOGGER_DATABASE_CONFIG_HPP__
#define __ASFM_LOGGER_DATABASE_CONFIG_HPP__

/**
 * ASFMLogger Database Layer POD Structures
 *
 * Pure data structures for database configuration following toolbox architecture.
 * No methods, pure data for maximum compatibility and testability.
 */

#include "structs/LogDataStructures.hpp"
#include <cstdint>

// =====================================================================================
// DATABASE CONNECTION DATA STRUCTURES
// =====================================================================================

/**
 * @brief POD structure for database connection configuration
 * Contains all information needed to establish database connections
 */
struct DatabaseConnectionConfig {
    uint32_t connection_id;                    ///< Unique connection identifier
    char server_name[256];                     ///< SQL Server instance name
    char database_name[256];                   ///< Database name
    char username[128];                        ///< Authentication username
    char password[256];                        ///< Authentication password (encrypted)
    char connection_string[1024];              ///< Full connection string

    // Connection settings
    DWORD connection_timeout_seconds;          ///< Connection timeout in seconds
    DWORD command_timeout_seconds;             ///< Command execution timeout
    bool use_windows_authentication;           ///< Whether to use Windows authentication
    bool enable_connection_pooling;            ///< Whether to use connection pooling
    bool enable_mars;                          ///< Multiple Active Result Sets

    // SSL and security
    bool encrypt_connection;                   ///< Whether to encrypt connection
    bool trust_server_certificate;             ///< Whether to trust server certificate
    char certificate_path[512];                ///< Path to client certificate

    // Performance settings
    size_t max_pool_size;                      ///< Maximum connections in pool
    size_t min_pool_size;                      ///< Minimum connections in pool
    DWORD connection_lifetime_seconds;         ///< Connection lifetime before recycling
    DWORD connection_idle_timeout_seconds;     ///< Idle timeout before closing connection

    // Retry and reliability
    DWORD retry_count;                         ///< Number of retry attempts
    DWORD retry_delay_ms;                      ///< Delay between retries
    bool enable_failover;                      ///< Whether to enable failover support

    // Monitoring
    bool enable_connection_monitoring;         ///< Whether to monitor connections
    DWORD monitoring_interval_seconds;         ///< Monitoring check interval

    // Lifecycle management
    DWORD created_time;                        ///< When configuration was created
    DWORD last_modified;                       ///< When configuration was last modified
    char created_by[128];                      ///< Who created this configuration
    bool is_active;                            ///< Whether this configuration is active

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for database connection state
 * Tracks the current state of a database connection
 */
struct DatabaseConnectionState {
    uint32_t connection_id;                    ///< Connection identifier
    DWORD connection_start_time;               ///< When connection was established
    DWORD last_activity_time;                  ///< Last time connection was used
    DWORD connection_duration_seconds;         ///< How long connection has been active

    // Performance metrics
    uint64_t total_commands_executed;          ///< Total SQL commands executed
    uint64_t total_rows_affected;              ///< Total rows affected by operations
    DWORD average_response_time_ms;            ///< Average command response time
    DWORD max_response_time_ms;                ///< Maximum observed response time

    // Connection health
    DWORD consecutive_failures;                ///< Number of consecutive failures
    DWORD total_failures;                      ///< Total failure count
    DWORD last_failure_time;                   ///< When last failure occurred
    char last_error_message[512];              ///< Last error message

    // Pool information
    bool is_from_pool;                         ///< Whether connection came from pool
    DWORD pool_wait_time_ms;                   ///< Time spent waiting for pooled connection
    DWORD time_in_pool_seconds;                ///< Time spent in connection pool

    // State flags
    bool is_connected;                         ///< Whether connection is currently active
    bool is_busy;                              ///< Whether connection is currently in use
    bool is_disposed;                          ///< Whether connection has been disposed
    bool has_transaction;                      ///< Whether connection has active transaction

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for database operation result
 * Contains the outcome of database operations
 */
struct DatabaseOperationResult {
    uint32_t operation_id;                     ///< Unique operation identifier
    DWORD start_time;                          ///< When operation started
    DWORD end_time;                            ///< When operation completed
    DWORD duration_ms;                         ///< Operation duration in milliseconds

    // Operation details
    char operation_type[64];                   ///< Type of operation ("INSERT", "SELECT", "UPDATE")
    char table_name[128];                      ///< Target table name
    uint64_t rows_affected;                    ///< Number of rows affected
    size_t data_size_bytes;                    ///< Size of data processed

    // Result status
    bool success;                              ///< Whether operation succeeded
    DWORD error_code;                          ///< Database error code (if failed)
    char error_message[512];                   ///< Error message (if failed)
    char sql_state[16];                        ///< SQL state code

    // Performance impact
    DWORD cpu_time_used_ms;                    ///< CPU time used for operation
    DWORD io_time_used_ms;                     ///< I/O time used for operation
    size_t memory_used_bytes;                  ///< Memory used for operation

    // Connection information
    uint32_t connection_id;                    ///< Connection used for operation
    DWORD connection_wait_time_ms;             ///< Time spent waiting for connection

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for database transaction information
 * Tracks active database transactions
 */
struct DatabaseTransactionInfo {
    uint32_t transaction_id;                   ///< Unique transaction identifier
    uint32_t connection_id;                    ///< Connection this transaction uses
    DWORD transaction_start_time;              ///< When transaction started
    DWORD transaction_duration_seconds;        ///< How long transaction has been active

    // Transaction properties
    char isolation_level[64];                  ///< Transaction isolation level
    bool is_distributed;                       ///< Whether transaction is distributed
    DWORD nested_transaction_depth;            ///< Depth of nested transactions

    // Transaction state
    bool is_active;                            ///< Whether transaction is currently active
    bool is_committed;                         ///< Whether transaction was committed
    bool is_rolled_back;                       ///< Whether transaction was rolled back
    DWORD commit_time;                         ///< When transaction was committed

    // Performance metrics
    uint64_t total_commands_in_transaction;    ///< Commands executed in this transaction
    uint64_t total_rows_affected;              ///< Rows affected in this transaction
    size_t total_data_size_bytes;              ///< Total data size in transaction

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for database schema information
 * Contains metadata about database tables and structure
 */
struct DatabaseSchemaInfo {
    uint32_t schema_id;                        ///< Unique schema identifier
    char table_name[128];                      ///< Table name
    char schema_name[128];                     ///< Schema name
    DWORD creation_time;                       ///< When table was created

    // Table structure
    size_t column_count;                       ///< Number of columns in table
    size_t row_count_estimate;                 ///< Estimated number of rows
    size_t table_size_bytes;                   ///< Size of table in bytes
    size_t index_size_bytes;                   ///< Size of indexes in bytes

    // Column information (first 10 columns)
    struct ColumnInfo {
        char column_name[128];                 ///< Column name
        char data_type[64];                    ///< SQL data type
        DWORD max_length;                      ///< Maximum length
        bool is_nullable;                      ///< Whether column allows nulls
        bool is_primary_key;                   ///< Whether column is primary key
        bool is_indexed;                       ///< Whether column has index
    } columns[10];

    // Index information
    size_t index_count;                        ///< Number of indexes
    char primary_key_columns[512];             ///< Primary key column list
    char clustered_index_columns[512];         ///< Clustered index columns

    // Maintenance information
    DWORD last_statistics_update;              ///< When statistics were last updated
    DWORD last_index_rebuild;                  ///< When indexes were last rebuilt
    DWORD fragmentation_percentage;            ///< Index fragmentation percentage

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

/**
 * @brief POD structure for database query execution plan
 * Contains information about how queries will be executed
 */
struct DatabaseQueryPlan {
    uint32_t plan_id;                          ///< Unique plan identifier
    char query_hash[64];                       ///< Hash of the query
    DWORD plan_creation_time;                  ///< When plan was created

    // Plan characteristics
    DWORD estimated_cost;                      ///< Estimated execution cost
    DWORD estimated_rows;                      ///< Estimated rows to be processed
    DWORD estimated_cpu_cost;                  ///< Estimated CPU cost
    DWORD estimated_io_cost;                   ///< Estimated I/O cost

    // Plan structure
    size_t operation_count;                    ///< Number of operations in plan
    char plan_type[64];                        ///< Type of execution plan
    bool uses_parallel_execution;              ///< Whether plan uses parallel execution
    DWORD degree_of_parallelism;               ///< Degree of parallelism

    // Index usage
    size_t indexes_used_count;                 ///< Number of indexes used
    char indexes_used[10][128];                ///< List of indexes used
    bool requires_table_scan;                  ///< Whether plan requires table scan

    // Performance warnings
    bool has_warnings;                         ///< Whether plan has performance warnings
    char warning_details[512];                 ///< Details of performance warnings

    // Future extensibility
    char reserved[256];                        ///< Reserved for future use
};

#endif // __ASFM_LOGGER_DATABASE_CONFIG_HPP__