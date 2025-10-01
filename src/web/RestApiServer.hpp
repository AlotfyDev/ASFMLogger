#ifndef __ASFM_LOGGER_REST_API_SERVER_HPP__
#define __ASFM_LOGGER_REST_API_SERVER_HPP__

/**
 * ASFMLogger REST API Server
 *
 * HTTP REST API server for accessing log data and managing logging configuration.
 * Provides web interface for log querying, statistics, and real-time monitoring.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/DatabaseConfiguration.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

// Forward declarations
struct httplib::Server;
struct LogMessageData;

class RestApiServer {
private:
    std::unique_ptr<httplib::Server> server_;
    int port_;
    bool is_running_;
    std::atomic<bool> should_stop_;
    std::thread server_thread_;
    mutable std::mutex server_mutex_;

    // API configuration
    std::string api_key_;
    bool authentication_enabled_;
    std::vector<std::string> allowed_origins_;
    DWORD request_timeout_seconds_;

    // Database connection for log queries
    DatabaseConnectionConfig database_config_;
    bool database_enabled_;

    // Real-time subscriptions
    std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> subscriptions_;
    std::mutex subscription_mutex_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    RestApiServer();

    /**
     * @brief Constructor with port configuration
     * @param port HTTP server port
     */
    explicit RestApiServer(int port);

    /**
     * @brief Constructor with full configuration
     * @param port HTTP server port
     * @param database_config Database configuration for log queries
     * @param enable_authentication Whether to enable API key authentication
     */
    RestApiServer(int port, const DatabaseConnectionConfig& database_config, bool enable_authentication = false);

    /**
     * @brief Destructor
     */
    ~RestApiServer();

    // =================================================================================
    // SERVER MANAGEMENT
    // =================================================================================

    /**
     * @brief Start the REST API server
     * @return true if server started successfully
     */
    bool start();

    /**
     * @brief Stop the REST API server
     * @return true if server stopped successfully
     */
    bool stop();

    /**
     * @brief Check if server is currently running
     * @return true if server is running
     */
    bool isRunning() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return is_running_;
    }

    /**
     * @brief Get server port
     * @return Server port number
     */
    int getPort() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return port_;
    }

    /**
     * @brief Set server port
     * @param port New port number
     * @return true if port was set successfully
     */
    bool setPort(int port);

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Set API key for authentication
     * @param api_key API key for authentication
     */
    void setApiKey(const std::string& api_key);

    /**
     * @brief Get current API key
     * @return Current API key
     */
    std::string getApiKey() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return api_key_;
    }

    /**
     * @brief Enable or disable authentication
     * @param enabled Whether authentication should be enabled
     */
    void setAuthenticationEnabled(bool enabled);

    /**
     * @brief Check if authentication is enabled
     * @return true if authentication is enabled
     */
    bool isAuthenticationEnabled() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return authentication_enabled_;
    }

    /**
     * @brief Add allowed CORS origin
     * @param origin Origin to allow
     */
    void addAllowedOrigin(const std::string& origin);

    /**
     * @brief Remove allowed CORS origin
     * @param origin Origin to remove
     * @return true if origin was found and removed
     */
    bool removeAllowedOrigin(const std::string& origin);

    /**
     * @brief Get all allowed origins
     * @return Vector of allowed origins
     */
    std::vector<std::string> getAllowedOrigins() const;

    /**
     * @brief Set request timeout
     * @param timeout_seconds Request timeout in seconds
     */
    void setRequestTimeout(DWORD timeout_seconds);

    /**
     * @brief Get request timeout
     * @return Request timeout in seconds
     */
    DWORD getRequestTimeout() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return request_timeout_seconds_;
    }

    // =================================================================================
    // DATABASE INTEGRATION
    // =================================================================================

    /**
     * @brief Set database configuration for log queries
     * @param config Database configuration
     * @return true if configuration was set successfully
     */
    bool setDatabaseConfig(const DatabaseConnectionConfig& config);

    /**
     * @brief Get current database configuration
     * @return Current database configuration
     */
    DatabaseConnectionConfig getDatabaseConfig() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return database_config_;
    }

    /**
     * @brief Enable database integration
     * @return true if database integration enabled successfully
     */
    bool enableDatabase();

    /**
     * @brief Disable database integration
     * @return true if database integration disabled successfully
     */
    bool disableDatabase();

    /**
     * @brief Check if database integration is enabled
     * @return true if database integration is enabled
     */
    bool isDatabaseEnabled() const {
        std::lock_guard<std::mutex> lock(server_mutex_);
        return database_enabled_;
    }

    // =================================================================================
    // API ENDPOINTS
    // =================================================================================

    /**
     * @brief Setup all API endpoints
     */
    void setupEndpoints();

    /**
     * @brief Setup health check endpoint
     */
    void setupHealthEndpoint();

    /**
     * @brief Setup log query endpoints
     */
    void setupLogQueryEndpoints();

    /**
     * @brief Setup statistics endpoints
     */
    void setupStatisticsEndpoints();

    /**
     * @brief Setup configuration endpoints
     */
    void setupConfigurationEndpoints();

    /**
     * @brief Setup real-time streaming endpoints
     */
    void setupStreamingEndpoints();

    /**
     * @brief Setup monitoring endpoints
     */
    void setupMonitoringEndpoints();

    // =================================================================================
    // AUTHENTICATION AND AUTHORIZATION
    // =================================================================================

    /**
     * @brief Authenticate API request
     * @param auth_header Authorization header value
     * @return true if authentication successful
     */
    bool authenticateRequest(const std::string& auth_header);

    /**
     * @brief Validate API key
     * @param api_key API key to validate
     * @return true if API key is valid
     */
    bool validateApiKey(const std::string& api_key);

    /**
     * @brief Check CORS origin
     * @param origin Origin to check
     * @return true if origin is allowed
     */
    bool isOriginAllowed(const std::string& origin);

    /**
     * @brief Generate API key
     * @param length Length of API key to generate
     * @return Generated API key
     */
    std::string generateApiKey(size_t length = 32);

    // =================================================================================
    // LOG QUERY API METHODS
    // =================================================================================

    /**
     * @brief Query logs by application
     * @param application_name Application name to filter by
     * @param limit Maximum number of logs to return
     * @param offset Offset for pagination
     * @return JSON response with log data
     */
    std::string queryLogsByApplication(const std::string& application_name, size_t limit = 100, size_t offset = 0);

    /**
     * @brief Query logs by time range
     * @param start_time Start timestamp
     * @param end_time End timestamp
     * @param limit Maximum number of logs to return
     * @param offset Offset for pagination
     * @return JSON response with log data
     */
    std::string queryLogsByTimeRange(DWORD start_time, DWORD end_time, size_t limit = 100, size_t offset = 0);

    /**
     * @brief Query logs by type
     * @param message_type Message type to filter by
     * @param limit Maximum number of logs to return
     * @param offset Offset for pagination
     * @return JSON response with log data
     */
    std::string queryLogsByType(LogMessageType message_type, size_t limit = 100, size_t offset = 0);

    /**
     * @brief Query logs with custom filters
     * @param filters Map of filter criteria
     * @param limit Maximum number of logs to return
     * @param offset Offset for pagination
     * @return JSON response with log data
     */
    std::string queryLogsWithFilters(const std::unordered_map<std::string, std::string>& filters,
                                    size_t limit = 100, size_t offset = 0);

    /**
     * @brief Get log statistics
     * @param time_range_hours Time range for statistics
     * @return JSON response with statistics
     */
    std::string getLogStatistics(DWORD time_range_hours = 24);

    /**
     * @brief Get application statistics
     * @return JSON response with application statistics
     */
    std::string getApplicationStatistics();

    // =================================================================================
    // REAL-TIME STREAMING
    // =================================================================================

    /**
     * @brief Subscribe to real-time log stream
     * @param client_id Unique client identifier
     * @param filters Subscription filters
     * @param callback Callback function for new messages
     * @return Subscription ID
     */
    std::string subscribeToLogStream(const std::string& client_id,
                                     const std::unordered_map<std::string, std::string>& filters,
                                     std::function<void(const std::string&)> callback);

    /**
     * @brief Unsubscribe from log stream
     * @param subscription_id Subscription ID to remove
     * @return true if subscription was found and removed
     */
    bool unsubscribeFromLogStream(const std::string& subscription_id);

    /**
     * @brief Broadcast message to all subscribers
     * @param message Message to broadcast
     * @param filters Filters to apply before broadcasting
     */
    void broadcastToSubscribers(const std::string& message, const std::unordered_map<std::string, std::string>& filters = {});

    /**
     * @brief Get active subscription count
     * @return Number of active subscriptions
     */
    size_t getActiveSubscriptionCount() const;

    // =================================================================================
    // MONITORING API METHODS
    // =================================================================================

    /**
     * @brief Get system health status
     * @return JSON response with health information
     */
    std::string getSystemHealth();

    /**
     * @brief Get performance metrics
     * @param time_range_minutes Time range for metrics
     * @return JSON response with performance metrics
     */
    std::string getPerformanceMetrics(DWORD time_range_minutes = 60);

    /**
     * @brief Get queue status
     * @return JSON response with queue information
     */
    std::string getQueueStatus();

    /**
     * @brief Get database status
     * @return JSON response with database information
     */
    std::string getDatabaseStatus();

    /**
     * @brief Get alert status
     * @return JSON response with alert information
     */
    std::string getAlertStatus();

    // =================================================================================
    // CONFIGURATION API METHODS
    // =================================================================================

    /**
     * @brief Get current configuration
     * @return JSON response with current configuration
     */
    std::string getConfiguration();

    /**
     * @brief Update configuration
     * @param config_json JSON configuration to apply
     * @return JSON response with update result
     */
    std::string updateConfiguration(const std::string& config_json);

    /**
     * @brief Reset configuration to defaults
     * @return JSON response with reset result
     */
    std::string resetConfiguration();

    /**
     * @brief Get available configuration templates
     * @return JSON response with available templates
     */
    std::string getConfigurationTemplates();

    /**
     * @brief Apply configuration template
     * @param template_name Template name to apply
     * @return JSON response with application result
     */
    std::string applyConfigurationTemplate(const std::string& template_name);

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Validate API request
     * @param request HTTP request object
     * @return true if request is valid
     */
    bool validateRequest(const httplib::Request& request);

    /**
     * @brief Generate JSON error response
     * @param error_code HTTP error code
     * @param error_message Error message
     * @return JSON error response
     */
    std::string generateErrorResponse(int error_code, const std::string& error_message);

    /**
     * @brief Generate JSON success response
     * @param data Response data
     * @return JSON success response
     */
    std::string generateSuccessResponse(const std::string& data);

    /**
     * @brief Parse query parameters from request
     * @param request HTTP request object
     * @return Map of query parameters
     */
    std::unordered_map<std::string, std::string> parseQueryParameters(const httplib::Request& request);

    /**
     * @brief Parse JSON body from request
     * @param request HTTP request object
     * @return Parsed JSON object
     */
    nlohmann::json parseJsonBody(const httplib::Request& request);

    /**
     * @brief Get server status information
     * @return Server status as formatted string
     */
    std::string getServerStatus() const;

    /**
     * @brief Get API documentation
     * @return OpenAPI/Swagger documentation as JSON
     */
    std::string getApiDocumentation();

private:
    // Private helper methods
    void serverLoop();
    void setupCorsHeaders();
    void setupAuthentication();
    void setupRateLimiting();
    bool initializeDatabaseConnection();
    void cleanupDatabaseConnection();
    std::string executeLogQuery(const std::string& query);
    std::vector<LogMessageData> parseLogQueryResults(const std::string& query_result);
    void handleNewLogMessage(const LogMessageData& message);
    void processSubscriptions(const LogMessageData& message);
    bool isRequestAuthenticated(const httplib::Request& request);
    std::string getClientIpAddress(const httplib::Request& request);
};

// =====================================================================================
// HTTP HANDLER FUNCTIONS
// =====================================================================================

/**
 * @brief Handle health check requests
 * @param request HTTP request
 * @param response HTTP response
 */
void HandleHealthCheck(const httplib::Request& request, httplib::Response& response);

/**
 * @brief Handle log query requests
 * @param request HTTP request
 * @param response HTTP response
 */
void HandleLogQuery(const httplib::Request& request, httplib::Response& response);

/**
 * @brief Handle statistics requests
 * @param request HTTP request
 * @param response HTTP response
 */
void HandleStatistics(const httplib::Request& request, httplib::Response& response);

/**
 * @brief Handle configuration requests
 * @param request HTTP request
 * @param response HTTP response
 */
void HandleConfiguration(const httplib::Request& request, httplib::Response& response);

/**
 * @brief Handle WebSocket upgrade requests
 * @param request HTTP request
 * @param response HTTP response
 */
void HandleWebSocketUpgrade(const httplib::Request& request, httplib::Response& response);

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

/**
 * @brief Create and start REST API server
 * @param port Server port
 * @param database_config Database configuration
 * @param enable_authentication Whether to enable authentication
 * @return REST API server instance
 */
std::unique_ptr<RestApiServer> CreateRestApiServer(int port,
                                                   const DatabaseConnectionConfig& database_config,
                                                   bool enable_authentication = false);

/**
 * @brief Start REST API server for application
 * @param application_name Name of the application
 * @param port Server port
 * @return true if server started successfully
 */
bool StartApplicationRestApi(const std::string& application_name, int port = 8080);

#endif // __ASFM_LOGGER_REST_API_SERVER_HPP__