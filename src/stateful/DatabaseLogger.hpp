#ifndef __ASFM_LOGGER_DATABASE_LOGGER_HPP__
#define __ASFM_LOGGER_DATABASE_LOGGER_HPP__

/**
 * ASFMLogger Stateful Database Logger
 *
 * Stateful wrapper class for database operations using toolbox internally.
 * Provides object-oriented interface for SQL Server integration.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/DatabaseConfiguration.hpp"
#include "toolbox/DatabaseToolbox.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

class DatabaseLogger {
private:
    DatabaseConnectionConfig connection_config_;
    uint32_t connection_id_;
    bool is_connected_;
    mutable std::mutex connection_mutex_;

    // Performance tracking
    std::atomic<uint64_t> total_operations_;
    std::atomic<uint64_t> successful_operations_;
    std::atomic<uint64_t> failed_operations_;
    std::atomic<DWORD> total_operation_time_ms_;

    // Connection management
    std::vector<uint32_t> active_connections_;
    size_t max_connections_;
    std::mutex connection_pool_mutex_;

    // Prepared statements
    std::unordered_map<std::string, void*> prepared_statements_;
    std::mutex prepared_statement_mutex_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    DatabaseLogger();

    /**
     * @brief Constructor with connection configuration
     * @param config Database connection configuration
     */
    explicit DatabaseLogger(const DatabaseConnectionConfig& config);

    /**
     * @brief Constructor with connection string
     * @param connection_string Full database connection string
     */
    explicit DatabaseLogger(const std::string& connection_string);

    /**
     * @brief Constructor with server and database details
     * @param server_name SQL Server instance name
     * @param database_name Database name
     * @param username Authentication username (optional)
     * @param password Authentication password (optional)
     * @param use_windows_auth Whether to use Windows authentication
     */
    DatabaseLogger(const std::string& server_name,
                   const std::string& database_name,
                   const std::string& username = "",
                   const std::string& password = "",
                   bool use_windows_auth = true);

    /**
     * @brief Copy constructor
     * @param other Database logger to copy from
     */
    DatabaseLogger(const DatabaseLogger& other);

    /**
     * @brief Move constructor
     * @param other Database logger to move from
     */
    DatabaseLogger(DatabaseLogger&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Database logger to assign from
     * @return Reference to this logger
     */
    DatabaseLogger& operator=(const DatabaseLogger& other);

    /**
     * @brief Move assignment operator
     * @param other Database logger to move from
     * @return Reference to this logger
     */
    DatabaseLogger& operator=(DatabaseLogger&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~DatabaseLogger();

    // =================================================================================
    // CONNECTION MANAGEMENT
    // =================================================================================

    /**
     * @brief Connect to database
     * @return true if connection successful
     */
    bool connect();

    /**
     * @brief Disconnect from database
     * @return true if disconnection successful
     */
    bool disconnect();

    /**
     * @brief Check if currently connected
     * @return true if connected to database
     */
    bool isConnected() const {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        return is_connected_;
    }

    /**
     * @brief Test database connection
     * @return true if connection test successful
     */
    bool testConnection();

    /**
     * @brief Get connection configuration
     * @return Current connection configuration
     */
    DatabaseConnectionConfig getConnectionConfig() const {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        return connection_config_;
    }

    /**
     * @brief Update connection configuration
     * @param config New connection configuration
     * @return true if configuration is valid and was updated
     */
    bool updateConnectionConfig(const DatabaseConnectionConfig& config);

    /**
     * @brief Get connection ID
     * @return Current connection identifier
     */
    uint32_t getConnectionId() const {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        return connection_id_;
    }

    // =================================================================================
    // SCHEMA MANAGEMENT
    // =================================================================================

    /**
     * @brief Check if log table exists
     * @param table_name Table name to check (optional, uses default if not specified)
     * @param schema_name Schema name (optional)
     * @return true if table exists
     */
    bool logTableExists(const std::string& table_name = "", const std::string& schema_name = "dbo");

    /**
     * @brief Create log table and supporting objects
     * @param table_name Name for the log table (optional, uses default if not specified)
     * @param schema_name Schema name (optional)
     * @return true if table creation successful
     */
    bool createLogTable(const std::string& table_name = "", const std::string& schema_name = "dbo");

    /**
     * @brief Get database schema information
     * @param table_name Table name to analyze (optional)
     * @param schema_name Schema name (optional)
     * @return Schema information
     */
    DatabaseSchemaInfo getSchemaInfo(const std::string& table_name = "",
                                    const std::string& schema_name = "dbo");

    /**
     * @brief Perform database maintenance
     * @param table_name Table to maintain (optional)
     * @param schema_name Schema name (optional)
     * @return true if maintenance successful
     */
    bool performMaintenance(const std::string& table_name = "", const std::string& schema_name = "dbo");

    // =================================================================================
    // MESSAGE PERSISTENCE
    // =================================================================================

    /**
     * @brief Insert single log message
     * @param message Message to insert
     * @param table_name Target table name (optional)
     * @param schema_name Schema name (optional)
     * @return Operation result with success/failure information
     */
    DatabaseOperationResult insertMessage(const LogMessageData& message,
                                         const std::string& table_name = "",
                                         const std::string& schema_name = "dbo");

    /**
     * @brief Insert multiple messages in batch
     * @param messages Vector of messages to insert
     * @param table_name Target table name (optional)
     * @param schema_name Schema name (optional)
     * @return Operation result with batch statistics
     */
    DatabaseOperationResult insertMessageBatch(const std::vector<LogMessageData>& messages,
                                              const std::string& table_name = "",
                                              const std::string& schema_name = "dbo");

    /**
     * @brief Insert message with automatic retry on failure
     * @param message Message to insert
     * @param max_retries Maximum number of retry attempts
     * @param table_name Target table name (optional)
     * @param schema_name Schema name (optional)
     * @return Operation result
     */
    DatabaseOperationResult insertMessageWithRetry(const LogMessageData& message,
                                                  DWORD max_retries = 3,
                                                  const std::string& table_name = "",
                                                  const std::string& schema_name = "dbo");

    /**
     * @brief Insert messages with automatic batching and retry
     * @param messages Vector of messages to insert
     * @param max_retries Maximum number of retry attempts
     * @param table_name Target table name (optional)
     * @param schema_name Schema name (optional)
     * @return Operation result
     */
    DatabaseOperationResult insertMessagesWithRetry(const std::vector<LogMessageData>& messages,
                                                   DWORD max_retries = 3,
                                                   const std::string& table_name = "",
                                                   const std::string& schema_name = "dbo");

    // =================================================================================
    // QUERY OPERATIONS
    // =================================================================================

    /**
     * @brief Query messages by application
     * @param application_name Application name to filter by
     * @param limit Maximum number of messages to return (optional)
     * @param table_name Source table name (optional)
     * @param schema_name Schema name (optional)
     * @return Vector of matching messages
     */
    std::vector<LogMessageData> queryByApplication(const std::string& application_name,
                                                  size_t limit = 1000,
                                                  const std::string& table_name = "",
                                                  const std::string& schema_name = "dbo");

    /**
     * @brief Query messages by time range
     * @param start_time Start of time range
     * @param end_time End of time range
     * @param limit Maximum number of messages to return (optional)
     * @param table_name Source table name (optional)
     * @param schema_name Schema name (optional)
     * @return Vector of matching messages
     */
    std::vector<LogMessageData> queryByTimeRange(DWORD start_time,
                                                DWORD end_time,
                                                size_t limit = 1000,
                                                const std::string& table_name = "",
                                                const std::string& schema_name = "dbo");

    /**
     * @brief Query messages by type
     * @param message_type Message type to filter by
     * @param limit Maximum number of messages to return (optional)
     * @param table_name Source table name (optional)
     * @param schema_name Schema name (optional)
     * @return Vector of matching messages
     */
    std::vector<LogMessageData> queryByMessageType(LogMessageType message_type,
                                                   size_t limit = 1000,
                                                   const std::string& table_name = "",
                                                   const std::string& schema_name = "dbo");

    /**
     * @brief Query messages with custom filters
     * @param filters SQL WHERE clause filters
     * @param order_by SQL ORDER BY clause (optional)
     * @param limit Maximum number of messages to return (optional)
     * @param table_name Source table name (optional)
     * @param schema_name Schema name (optional)
     * @return Vector of matching messages
     */
    std::vector<LogMessageData> queryWithFilters(const std::string& filters,
                                                const std::string& order_by = "",
                                                size_t limit = 1000,
                                                const std::string& table_name = "",
                                                const std::string& schema_name = "dbo");

    /**
     * @brief Get message statistics from database
     * @param time_range_hours Time range in hours (optional)
     * @param table_name Source table name (optional)
     * @param schema_name Schema name (optional)
     * @return Vector of statistic pairs (name, value)
     */
    std::vector<std::pair<std::string, uint64_t>> getMessageStatistics(
        DWORD time_range_hours = 0,
        const std::string& table_name = "",
        const std::string& schema_name = "dbo");

    // =================================================================================
    // TRANSACTION MANAGEMENT
    // =================================================================================

    /**
     * @brief Begin database transaction
     * @param isolation_level Transaction isolation level (optional)
     * @return true if transaction started successfully
     */
    bool beginTransaction(const std::string& isolation_level = "");

    /**
     * @brief Commit current transaction
     * @return true if transaction committed successfully
     */
    bool commitTransaction();

    /**
     * @brief Rollback current transaction
     * @return true if transaction rolled back successfully
     */
    bool rollbackTransaction();

    /**
     * @brief Create savepoint in current transaction
     * @param savepoint_name Name of the savepoint
     * @return true if savepoint created successfully
     */
    bool createSavepoint(const std::string& savepoint_name);

    /**
     * @brief Rollback to savepoint
     * @param savepoint_name Name of the savepoint
     * @return true if rollback successful
     */
    bool rollbackToSavepoint(const std::string& savepoint_name);

    /**
     * @brief Check if transaction is currently active
     * @return true if transaction is active
     */
    bool hasActiveTransaction() const;

    // =================================================================================
    // PREPARED STATEMENTS
    // =================================================================================

    /**
     * @brief Prepare INSERT statement for repeated use
     * @param statement_name Name for the prepared statement
     * @param table_name Target table name (optional)
     * @param schema_name Schema name (optional)
     * @return true if statement prepared successfully
     */
    bool prepareInsertStatement(const std::string& statement_name,
                               const std::string& table_name = "",
                               const std::string& schema_name = "dbo");

    /**
     * @brief Execute prepared INSERT statement
     * @param statement_name Name of the prepared statement
     * @param message Message to insert
     * @return Operation result
     */
    DatabaseOperationResult executePreparedInsert(const std::string& statement_name,
                                                 const LogMessageData& message);

    /**
     * @brief Prepare SELECT statement for repeated use
     * @param statement_name Name for the prepared statement
     * @param filters Query filters
     * @param table_name Source table name (optional)
     * @param schema_name Schema name (optional)
     * @return true if statement prepared successfully
     */
    bool prepareSelectStatement(const std::string& statement_name,
                               const std::string& filters = "",
                               const std::string& table_name = "",
                               const std::string& schema_name = "dbo");

    /**
     * @brief Execute prepared SELECT statement
     * @param statement_name Name of the prepared statement
     * @param limit Maximum number of messages to return (optional)
     * @return Vector of matching messages
     */
    std::vector<LogMessageData> executePreparedSelect(const std::string& statement_name,
                                                     size_t limit = 1000);

    /**
     * @brief Release prepared statement
     * @param statement_name Name of the prepared statement to release
     * @return true if statement was found and released
     */
    bool releasePreparedStatement(const std::string& statement_name);

    /**
     * @brief Release all prepared statements
     */
    void releaseAllPreparedStatements();

    // =================================================================================
    // PERFORMANCE MONITORING
    // =================================================================================

    /**
     * @brief Get database logger statistics
     * @return Current performance statistics
     */
    std::string getStatistics() const;

    /**
     * @brief Get performance metrics
     * @return Vector of performance metrics
     */
    std::vector<std::pair<std::string, double>> getPerformanceMetrics() const;

    /**
     * @brief Get total operations performed
     * @return Total number of database operations
     */
    uint64_t getTotalOperations() const {
        return total_operations_;
    }

    /**
     * @brief Get successful operations count
     * @return Number of successful operations
     */
    uint64_t getSuccessfulOperations() const {
        return successful_operations_;
    }

    /**
     * @brief Get failed operations count
     * @return Number of failed operations
     */
    uint64_t getFailedOperations() const {
        return failed_operations_;
    }

    /**
     * @brief Get average operation time
     * @return Average operation time in milliseconds
     */
    DWORD getAverageOperationTime() const;

    /**
     * @brief Reset performance statistics
     */
    void resetStatistics();

    // =================================================================================
    // CONNECTION POOL MANAGEMENT
    // =================================================================================

    /**
     * @brief Set maximum number of connections in pool
     * @param max_connections Maximum connections
     */
    void setMaxConnections(size_t max_connections);

    /**
     * @brief Get current number of active connections
     * @return Number of active connections
     */
    size_t getActiveConnectionCount() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connection_pool_mutex_));
        return active_connections_.size();
    }

    /**
     * @brief Get maximum allowed connections
     * @return Maximum connections in pool
     */
    size_t getMaxConnections() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connection_pool_mutex_));
        return max_connections_;
    }

    /**
     * @brief Check if connection pool is available
     * @return true if connections are available in pool
     */
    bool isConnectionAvailable() const;

    /**
     * @brief Wait for connection to become available
     * @param timeout_ms Timeout in milliseconds
     * @return true if connection became available
     */
    bool waitForConnection(DWORD timeout_ms);

    // =================================================================================
    // MAINTENANCE OPERATIONS
    // =================================================================================

    /**
     * @brief Clean up old log records
     * @param retention_days Number of days to retain
     * @param table_name Table to clean up (optional)
     * @param schema_name Schema name (optional)
     * @return Number of records cleaned up
     */
    uint64_t cleanupOldRecords(DWORD retention_days,
                              const std::string& table_name = "",
                              const std::string& schema_name = "dbo");

    /**
     * @brief Update table statistics
     * @param table_name Table to update statistics for (optional)
     * @param schema_name Schema name (optional)
     * @return true if statistics updated successfully
     */
    bool updateStatistics(const std::string& table_name = "", const std::string& schema_name = "dbo");

    /**
     * @brief Rebuild indexes for better performance
     * @param table_name Table to rebuild indexes for (optional)
     * @param schema_name Schema name (optional)
     * @return true if indexes rebuilt successfully
     */
    bool rebuildIndexes(const std::string& table_name = "", const std::string& schema_name = "dbo");

    /**
     * @brief Perform comprehensive database maintenance
     * @param table_name Table to maintain (optional)
     * @param schema_name Schema name (optional)
     * @return true if maintenance completed successfully
     */
    bool performComprehensiveMaintenance(const std::string& table_name = "",
                                       const std::string& schema_name = "dbo");

    // =================================================================================
    // ERROR HANDLING AND DIAGNOSTICS
    // =================================================================================

    /**
     * @brief Get last error message
     * @return Last error message from database operations
     */
    std::string getLastError() const;

    /**
     * @brief Check if last operation had retryable error
     * @return true if last error can be retried
     */
    bool hasRetryableError() const;

    /**
     * @brief Get suggested retry delay for last error
     * @return Suggested retry delay in milliseconds
     */
    DWORD getRetryDelay() const;

    /**
     * @brief Execute diagnostic queries
     * @return Diagnostic information as formatted string
     */
    std::string runDiagnostics();

    /**
     * @brief Analyze query performance
     * @param query SQL query to analyze
     * @return Query plan information
     */
    DatabaseQueryPlan analyzeQuery(const std::string& query);

    // =================================================================================
    // CONFIGURATION AND OPTIMIZATION
    // =================================================================================

    /**
     * @brief Optimize for high-throughput scenarios
     * @return true if optimization successful
     */
    bool optimizeForHighThroughput();

    /**
     * @brief Optimize for low-latency scenarios
     * @return true if optimization successful
     */
    bool optimizeForLowLatency();

    /**
     * @brief Optimize for balanced performance
     * @return true if optimization successful
     */
    bool optimizeForBalanced();

    /**
     * @brief Set performance profile
     * @param profile Performance profile ("HIGH_THROUGHPUT", "LOW_LATENCY", "BALANCED")
     * @return true if profile was set successfully
     */
    bool setPerformanceProfile(const std::string& profile);

    /**
     * @brief Get current performance profile
     * @return Current performance profile
     */
    std::string getPerformanceProfile() const;

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Validate current configuration
     * @return true if configuration is valid
     */
    bool validateConfiguration() const;

    /**
     * @brief Clone this database logger
     * @return New DatabaseLogger with same configuration
     */
    DatabaseLogger clone() const;

    /**
     * @brief Swap contents with another logger
     * @param other Logger to swap with
     */
    void swap(DatabaseLogger& other) noexcept;

    /**
     * @brief Clear all prepared statements and reset state
     */
    void clear();

    /**
     * @brief Check if logger is properly configured
     * @return true if logger has valid configuration
     */
    bool isConfigured() const;

    /**
     * @brief Get logger status as formatted string
     * @return Status information
     */
    std::string getStatus() const;

private:
    // Private helper methods
    bool initializeConnection();
    void cleanupConnection();
    void updatePerformanceStatistics(const DatabaseOperationResult& result);
    std::string generateDefaultTableName() const;
    bool ensureTableExists(const std::string& table_name, const std::string& schema_name);
    DWORD getCurrentTimestamp() const;
};

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

/**
 * @brief Create database logger for application
 * @param application_name Name of the application
 * @param server_name SQL Server instance name
 * @param database_name Database name
 * @return New DatabaseLogger for the application
 */
DatabaseLogger CreateApplicationDatabaseLogger(const std::string& application_name,
                                              const std::string& server_name,
                                              const std::string& database_name);

/**
 * @brief Output stream operator for DatabaseLogger
 * @param os Output stream
 * @param logger Logger to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& os, const DatabaseLogger& logger);

#endif // __ASFM_LOGGER_DATABASE_LOGGER_HPP__