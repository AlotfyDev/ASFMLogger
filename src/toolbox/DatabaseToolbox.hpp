#ifndef __ASFM_LOGGER_DATABASE_TOOLBOX_HPP__
#define __ASFM_LOGGER_DATABASE_TOOLBOX_HPP__

/**
 * ASFMLogger Database Toolbox
 *
 * Static methods for database operations following toolbox architecture.
 * Pure functions for SQL Server integration and data persistence.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/DatabaseConfiguration.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations
struct LogMessageData;

class DatabaseToolbox {
private:
    // Only static variables allowed in toolbox
    static std::unordered_map<uint32_t, DatabaseConnectionConfig> connection_configs_;
    static std::unordered_map<uint32_t, DatabaseConnectionState> connection_states_;
    static std::unordered_map<std::string, uint32_t> application_connection_map_;

public:
    // =================================================================================
    // CONNECTION MANAGEMENT
    // =================================================================================

    /**
     * @brief Generate unique connection identifier
     * @return Unique connection ID
     */
    static uint32_t GenerateConnectionId();

    /**
     * @brief Create database connection configuration
     * @param server_name SQL Server instance name
     * @param database_name Database name
     * @param username Authentication username
     * @param password Authentication password
     * @param use_windows_auth Whether to use Windows authentication
     * @return Connection configuration
     */
    static DatabaseConnectionConfig CreateConnectionConfig(
        const std::string& server_name,
        const std::string& database_name,
        const std::string& username = "",
        const std::string& password = "",
        bool use_windows_auth = true);

    /**
     * @brief Build connection string from configuration
     * @param config Connection configuration
     * @return Formatted connection string
     */
    static std::string BuildConnectionString(const DatabaseConnectionConfig& config);

    /**
     * @brief Validate connection configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    static bool ValidateConnectionConfig(const DatabaseConnectionConfig& config);

    /**
     * @brief Test database connection
     * @param config Connection configuration to test
     * @return true if connection test successful
     */
    static bool TestConnection(const DatabaseConnectionConfig& config);

    /**
     * @brief Initialize connection state tracking
     * @param connection_id Connection identifier
     * @param config Connection configuration
     * @return true if initialization successful
     */
    static bool InitializeConnectionState(uint32_t connection_id, const DatabaseConnectionConfig& config);

    // =================================================================================
    // SCHEMA MANAGEMENT
    // =================================================================================

    /**
     * @brief Generate SQL script for log messages table creation
     * @param table_name Name for the log table
     * @param schema_name Schema name (optional)
     * @return SQL script for table creation
     */
    static std::string GenerateLogTableScript(const std::string& table_name,
                                             const std::string& schema_name = "dbo");

    /**
     * @brief Generate SQL script for indexes on log table
     * @param table_name Name of the log table
     * @param schema_name Schema name (optional)
     * @return SQL script for index creation
     */
    static std::string GenerateLogIndexesScript(const std::string& table_name,
                                               const std::string& schema_name = "dbo");

    /**
     * @brief Generate SQL script for stored procedures
     * @param table_name Name of the log table
     * @param schema_name Schema name (optional)
     * @return SQL script for stored procedure creation
     */
    static std::string GenerateStoredProceduresScript(const std::string& table_name,
                                                     const std::string& schema_name = "dbo");

    /**
     * @brief Check if log table exists
     * @param connection_config Connection configuration
     * @param table_name Table name to check
     * @param schema_name Schema name (optional)
     * @return true if table exists
     */
    static bool CheckLogTableExists(const DatabaseConnectionConfig& connection_config,
                                   const std::string& table_name,
                                   const std::string& schema_name = "dbo");

    /**
     * @brief Create log table and supporting objects
     * @param connection_config Connection configuration
     * @param table_name Name for the log table
     * @param schema_name Schema name (optional)
     * @return true if table creation successful
     */
    static bool CreateLogTable(const DatabaseConnectionConfig& connection_config,
                              const std::string& table_name,
                              const std::string& schema_name = "dbo");

    /**
     * @brief Get database schema information
     * @param connection_config Connection configuration
     * @param table_name Table name to analyze
     * @param schema_name Schema name (optional)
     * @return Schema information
     */
    static DatabaseSchemaInfo GetSchemaInfo(const DatabaseConnectionConfig& connection_config,
                                           const std::string& table_name,
                                           const std::string& schema_name = "dbo");

    // =================================================================================
    // MESSAGE INSERTION
    // =================================================================================

    /**
     * @brief Generate SQL INSERT statement for log message
     * @param message Message to insert
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return SQL INSERT statement
     */
    static std::string GenerateInsertStatement(const LogMessageData& message,
                                              const std::string& table_name,
                                              const std::string& schema_name = "dbo");

    /**
     * @brief Generate parameterized INSERT statement for prepared statements
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return Parameterized INSERT statement
     */
    static std::string GenerateParameterizedInsert(const std::string& table_name,
                                                  const std::string& schema_name = "dbo");

    /**
     * @brief Convert LogMessageData to parameter values for insertion
     * @param message Message to convert
     * @return Vector of parameter values
     */
    static std::vector<std::string> MessageToParameterValues(const LogMessageData& message);

    /**
     * @brief Convert LogMessageData to column values for direct insertion
     * @param message Message to convert
     * @return Vector of column values
     */
    static std::vector<std::string> MessageToColumnValues(const LogMessageData& message);

    /**
     * @brief Insert single message into database
     * @param connection_config Connection configuration
     * @param message Message to insert
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return Operation result
     */
    static DatabaseOperationResult InsertMessage(const DatabaseConnectionConfig& connection_config,
                                                const LogMessageData& message,
                                                const std::string& table_name,
                                                const std::string& schema_name = "dbo");

    // =================================================================================
    // BATCH INSERTION
    // =================================================================================

    /**
     * @brief Generate batch INSERT statement for multiple messages
     * @param messages Vector of messages to insert
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return SQL batch INSERT statement
     */
    static std::string GenerateBatchInsertStatement(const std::vector<LogMessageData>& messages,
                                                   const std::string& table_name,
                                                   const std::string& schema_name = "dbo");

    /**
     * @brief Insert multiple messages in batch
     * @param connection_config Connection configuration
     * @param messages Vector of messages to insert
     * @param table_name Target table name
     * @param schema_name Schema name (optional)
     * @return Operation result with batch statistics
     */
    static DatabaseOperationResult InsertMessageBatch(const DatabaseConnectionConfig& connection_config,
                                                     const std::vector<LogMessageData>& messages,
                                                     const std::string& table_name,
                                                     const std::string& schema_name = "dbo");

    /**
     * @brief Calculate optimal batch size for current conditions
     * @param average_message_size Average size of messages in bytes
     * @param available_memory_mb Available memory in MB
     * @param network_latency_ms Network latency in milliseconds
     * @return Optimal batch size
     */
    static size_t CalculateOptimalBatchSize(size_t average_message_size,
                                          size_t available_memory_mb,
                                          DWORD network_latency_ms);

    // =================================================================================
    // QUERY OPERATIONS
    // =================================================================================

    /**
     * @brief Generate SELECT query for log messages
     * @param table_name Source table name
     * @param schema_name Schema name (optional)
     * @param filters Query filters (optional)
     * @param order_by Order by clause (optional)
     * @param limit Maximum number of rows (optional)
     * @return SQL SELECT statement
     */
    static std::string GenerateSelectQuery(const std::string& table_name,
                                          const std::string& schema_name = "dbo",
                                          const std::string& filters = "",
                                          const std::string& order_by = "",
                                          size_t limit = 0);

    /**
     * @brief Generate query to filter by application
     * @param application_name Application name to filter by
     * @param table_name Source table name
     * @param schema_name Schema name (optional)
     * @return SQL query string
     */
    static std::string GenerateApplicationFilterQuery(const std::string& application_name,
                                                     const std::string& table_name,
                                                     const std::string& schema_name = "dbo");

    /**
     * @brief Generate query to filter by time range
     * @param start_time Start of time range
     * @param end_time End of time range
     * @param table_name Source table name
     * @param schema_name Schema name (optional)
     * @return SQL query string
     */
    static std::string GenerateTimeRangeQuery(DWORD start_time,
                                             DWORD end_time,
                                             const std::string& table_name,
                                             const std::string& schema_name = "dbo");

    /**
     * @brief Generate query to filter by message type
     * @param message_type Message type to filter by
     * @param table_name Source table name
     * @param schema_name Schema name (optional)
     * @return SQL query string
     */
    static std::string GenerateMessageTypeQuery(LogMessageType message_type,
                                               const std::string& table_name,
                                               const std::string& schema_name = "dbo");

    /**
     * @brief Generate query for message statistics
     * @param table_name Source table name
     * @param schema_name Schema name (optional)
     * @param time_range_hours Time range in hours (optional)
     * @return SQL query for statistics
     */
    static std::string GenerateStatisticsQuery(const std::string& table_name,
                                              const std::string& schema_name = "dbo",
                                              DWORD time_range_hours = 0);

    // =================================================================================
    // DATA CONVERSION
    // =================================================================================

    /**
     * @brief Convert LogMessageData to database column map
     * @param message Message to convert
     * @return Map of column names to values
     */
    static std::unordered_map<std::string, std::string> MessageToColumnMap(const LogMessageData& message);

    /**
     * @brief Convert database row to LogMessageData
     * @param row_data Database row data
     * @return Reconstructed LogMessageData
     */
    static LogMessageData RowDataToMessage(const std::unordered_map<std::string, std::string>& row_data);

    /**
     * @brief Escape string for safe SQL insertion
     * @param input String to escape
     * @return SQL-safe escaped string
     */
    static std::string EscapeSqlString(const std::string& input);

    /**
     * @brief Convert Unix timestamp to SQL datetime string
     * @param unix_timestamp Unix timestamp
     * @return SQL datetime string
     */
    static std::string UnixTimestampToSqlDateTime(DWORD unix_timestamp);

    /**
     * @brief Convert SQL datetime string to Unix timestamp
     * @param sql_datetime SQL datetime string
     * @return Unix timestamp
     */
    static DWORD SqlDateTimeToUnixTimestamp(const std::string& sql_datetime);

    // =================================================================================
    // TRANSACTION MANAGEMENT
    // =================================================================================

    /**
     * @brief Generate BEGIN TRANSACTION statement
     * @param isolation_level Transaction isolation level (optional)
     * @return SQL BEGIN TRANSACTION statement
     */
    static std::string GenerateBeginTransaction(const std::string& isolation_level = "");

    /**
     * @brief Generate COMMIT TRANSACTION statement
     * @return SQL COMMIT statement
     */
    static std::string GenerateCommitTransaction();

    /**
     * @brief Generate ROLLBACK TRANSACTION statement
     * @return SQL ROLLBACK statement
     */
    static std::string GenerateRollbackTransaction();

    /**
     * @brief Generate SAVE TRANSACTION statement
     * @param savepoint_name Name of the savepoint
     * @return SQL SAVE TRANSACTION statement
     */
    static std::string GenerateSaveTransaction(const std::string& savepoint_name);

    /**
     * @brief Generate ROLLBACK TO SAVEPOINT statement
     * @param savepoint_name Name of the savepoint
     * @return SQL ROLLBACK TO SAVEPOINT statement
     */
    static std::string GenerateRollbackToSavepoint(const std::string& savepoint_name);

    // =================================================================================
    // PERFORMANCE OPTIMIZATION
    // =================================================================================

    /**
     * @brief Generate query execution plan analysis
     * @param connection_config Connection configuration
     * @param query SQL query to analyze
     * @return Query plan information
     */
    static DatabaseQueryPlan AnalyzeQueryPlan(const DatabaseConnectionConfig& connection_config,
                                             const std::string& query);

    /**
     * @brief Generate index optimization suggestions
     * @param schema_info Database schema information
     * @return Vector of optimization suggestions
     */
    static std::vector<std::string> GenerateIndexOptimizationSuggestions(const DatabaseSchemaInfo& schema_info);

    /**
     * @brief Calculate optimal connection pool size
     * @param expected_concurrent_operations Expected concurrent database operations
     * @param average_operation_time_ms Average operation time in milliseconds
     * @param system_memory_gb Available system memory in GB
     * @return Optimal pool size
     */
    static size_t CalculateOptimalPoolSize(size_t expected_concurrent_operations,
                                          DWORD average_operation_time_ms,
                                          size_t system_memory_gb);

    /**
     * @brief Generate connection string with performance optimizations
     * @param base_config Base connection configuration
     * @param performance_profile Performance profile ("HIGH_THROUGHPUT", "LOW_LATENCY", "BALANCED")
     * @return Optimized connection string
     */
    static std::string GenerateOptimizedConnectionString(const DatabaseConnectionConfig& base_config,
                                                        const std::string& performance_profile);

    // =================================================================================
    // MAINTENANCE OPERATIONS
    // =================================================================================

    /**
     * @brief Generate table maintenance script
     * @param table_name Table to maintain
     * @param schema_name Schema name (optional)
     * @return SQL maintenance script
     */
    static std::string GenerateMaintenanceScript(const std::string& table_name,
                                                const std::string& schema_name = "dbo");

    /**
     * @brief Generate log cleanup script
     * @param table_name Table to clean up
     * @param schema_name Schema name (optional)
     * @param retention_days Number of days to retain
     * @return SQL cleanup script
     */
    static std::string GenerateCleanupScript(const std::string& table_name,
                                            const std::string& schema_name,
                                            DWORD retention_days);

    /**
     * @brief Generate statistics update script
     * @param table_name Table to update statistics for
     * @param schema_name Schema name (optional)
     * @return SQL statistics update script
     */
    static std::string GenerateUpdateStatisticsScript(const std::string& table_name,
                                                     const std::string& schema_name = "dbo");

    /**
     * @brief Generate index rebuild script
     * @param table_name Table to rebuild indexes for
     * @param schema_name Schema name (optional)
     * @return SQL index rebuild script
     */
    static std::string GenerateRebuildIndexesScript(const std::string& table_name,
                                                   const std::string& schema_name = "dbo");

    // =================================================================================
    // ERROR HANDLING AND DIAGNOSTICS
    // =================================================================================

    /**
     * @brief Parse SQL Server error message
     * @param error_message Raw error message from SQL Server
     * @return Parsed error information
     */
    static std::unordered_map<std::string, std::string> ParseSqlError(const std::string& error_message);

    /**
     * @brief Generate user-friendly error message
     * @param sql_error Parsed SQL error information
     * @return User-friendly error message
     */
    static std::string GenerateUserFriendlyError(const std::unordered_map<std::string, std::string>& sql_error);

    /**
     * @brief Check if error is retryable
     * @param sql_error Parsed SQL error information
     * @return true if error can be retried
     */
    static bool IsRetryableError(const std::unordered_map<std::string, std::string>& sql_error);

    /**
     * @brief Get suggested retry delay for error
     * @param sql_error Parsed SQL error information
     * @return Suggested retry delay in milliseconds
     */
    static DWORD GetRetryDelayForError(const std::unordered_map<std::string, std::string>& sql_error);

    /**
     * @brief Generate diagnostic query for troubleshooting
     * @param table_name Table to diagnose
     * @param schema_name Schema name (optional)
     * @return SQL diagnostic query
     */
    static std::string GenerateDiagnosticQuery(const std::string& table_name,
                                              const std::string& schema_name = "dbo");

    // =================================================================================
    // CONFIGURATION AND DEFAULTS
    // =================================================================================

    /**
     * @brief Create default database configuration for logging
     * @param server_name SQL Server instance name
     * @param database_name Database name
     * @return Default database configuration
     */
    static DatabaseConnectionConfig CreateDefaultLoggingConfig(const std::string& server_name,
                                                              const std::string& database_name);

    /**
     * @brief Create high-performance database configuration
     * @param server_name SQL Server instance name
     * @param database_name Database name
     * @return High-performance database configuration
     */
    static DatabaseConnectionConfig CreateHighPerformanceConfig(const std::string& server_name,
                                                               const std::string& database_name);

    /**
     * @brief Create connection configuration from connection string
     * @param connection_string Full connection string
     * @return Parsed connection configuration
     */
    static DatabaseConnectionConfig ParseConnectionString(const std::string& connection_string);

    /**
     * @brief Validate database operation result
     * @param result Operation result to validate
     * @return true if result indicates success
     */
    static bool ValidateOperationResult(const DatabaseOperationResult& result);

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Convert operation result to string
     * @param result Operation result to format
     * @return Human-readable operation description
     */
    static std::string OperationResultToString(const DatabaseOperationResult& result);

    /**
     * @brief Convert connection config to string
     * @param config Connection configuration to format
     * @param mask_password Whether to mask password in output
     * @return Human-readable configuration description
     */
    static std::string ConnectionConfigToString(const DatabaseConnectionConfig& config,
                                               bool mask_password = true);

    /**
     * @brief Convert connection state to string
     * @param state Connection state to format
     * @return Human-readable state description
     */
    static std::string ConnectionStateToString(const DatabaseConnectionState& state);

    /**
     * @brief Get current timestamp for database operations
     * @return Current timestamp
     */
    static DWORD GetCurrentTimestamp();

public:
    // =================================================================================
    // PUBLIC HELPER METHODS
    // =================================================================================

    /**
     * @brief Generate unique operation identifier
     * @return Unique operation ID
     */
    static uint32_t GenerateOperationId();

    /**
     * @brief Generate unique schema identifier
     * @return Unique schema ID
     */
    static uint32_t GenerateSchemaId();

    /**
     * @brief Generate unique plan identifier
     * @return Unique plan ID
     */
    static uint32_t GeneratePlanId();

private:
    // Private helper methods
    static std::string FormatSqlDateTime(DWORD unix_timestamp);
    static DWORD ParseSqlDateTime(const std::string& sql_datetime);
    static std::string GenerateColumnList(const std::string& table_name, const std::string& schema_name);
    static std::string GenerateParameterList(size_t parameter_count);
    static std::vector<std::string> ParseCommaSeparatedList(const std::string& input);
    static bool IsValidTableName(const std::string& table_name);
    static bool IsValidSchemaName(const std::string& schema_name);
    static void InitializeDefaultConfigurations();
    static bool IsInitialized();
};

#endif // __ASFM_LOGGER_DATABASE_TOOLBOX_HPP__