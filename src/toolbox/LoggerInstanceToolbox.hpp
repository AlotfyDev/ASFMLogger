#ifndef __ASFM_LOGGER_INSTANCE_TOOLBOX_HPP__
#define __ASFM_LOGGER_INSTANCE_TOOLBOX_HPP__

/**
 * ASFMLogger Instance Management Toolbox
 *
 * Static methods for logger instance management following toolbox architecture.
 * Pure functions for instance lifecycle and coordination.
 */

#include "structs/LoggerInstanceData.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>

// Forward declarations
struct LoggerInstanceData;
struct InstanceStatistics;
struct InstanceConfiguration;

class LoggerInstanceToolbox {
private:
    // Only static variables allowed in toolbox
    static std::atomic<uint32_t> global_instance_id_counter_;
    static std::mutex instance_management_mutex_;

public:
    // =================================================================================
    // INSTANCE ID GENERATION
    // =================================================================================

    /**
     * @brief Generate a unique instance ID
     * @return Unique 32-bit instance identifier
     */
    static uint32_t GenerateInstanceId();

    /**
     * @brief Reset the instance ID counter (primarily for testing)
     */
    static void ResetInstanceIdCounter();

    // =================================================================================
    // INSTANCE CREATION
    // =================================================================================

    /**
     * @brief Create a new logger instance data structure
     * @param application_name Name of the application
     * @param process_name Name of the process (optional)
     * @param instance_name Human-readable instance name (optional)
     * @return Configured LoggerInstanceData structure
     */
    static LoggerInstanceData CreateInstance(
        const std::string& application_name,
        const std::string& process_name = "",
        const std::string& instance_name = "");

    /**
     * @brief Initialize instance with system information
     * @param instance Instance data to initialize
     * @return true if initialization successful
     */
    static bool InitializeInstance(LoggerInstanceData& instance);

    /**
     * @brief Set instance application information
     * @param instance Instance data to modify
     * @param application_name Application name
     * @param process_name Process name (optional)
     * @return true if successful
     */
    static bool SetApplicationInfo(LoggerInstanceData& instance,
                                  const std::string& application_name,
                                  const std::string& process_name = "");

    /**
     * @brief Set instance identification
     * @param instance Instance data to modify
     * @param instance_name Human-readable instance name
     * @return true if successful
     */
    static bool SetInstanceName(LoggerInstanceData& instance, const std::string& instance_name);

    // =================================================================================
    // INSTANCE VALIDATION
    // =================================================================================

    /**
     * @brief Validate instance data structure
     * @param instance Instance data to validate
     * @return true if instance appears valid
     */
    static bool ValidateInstance(const LoggerInstanceData& instance);

    /**
     * @brief Check if instance has valid application information
     * @param instance Instance data to check
     * @return true if application info is set
     */
    static bool HasApplicationInfo(const LoggerInstanceData& instance);

    /**
     * @brief Check if instance is active (recent activity)
     * @param instance Instance data to check
     * @param max_idle_seconds Maximum idle time in seconds
     * @return true if instance is considered active
     */
    static bool IsInstanceActive(const LoggerInstanceData& instance, DWORD max_idle_seconds = 300);

    // =================================================================================
    // INSTANCE INFORMATION EXTRACTION
    // =================================================================================

    /**
     * @brief Get instance ID
     * @param instance Instance data to read
     * @return Instance ID
     */
    static uint32_t GetInstanceId(const LoggerInstanceData& instance);

    /**
     * @brief Get application name
     * @param instance Instance data to read
     * @return Application name string
     */
    static std::string GetApplicationName(const LoggerInstanceData& instance);

    /**
     * @brief Get process name
     * @param instance Instance data to read
     * @return Process name string
     */
    static std::string GetProcessName(const LoggerInstanceData& instance);

    /**
     * @brief Get instance name
     * @param instance Instance data to read
     * @return Instance name string
     */
    static std::string GetInstanceName(const LoggerInstanceData& instance);

    /**
     * @brief Get process ID
     * @param instance Instance data to read
     * @return Process ID
     */
    static DWORD GetProcessId(const LoggerInstanceData& instance);

    /**
     * @brief Get creation time
     * @param instance Instance data to read
     * @return Creation timestamp
     */
    static DWORD GetCreationTime(const LoggerInstanceData& instance);

    /**
     * @brief Get last activity time
     * @param instance Instance data to read
     * @return Last activity timestamp
     */
    static DWORD GetLastActivity(const LoggerInstanceData& instance);

    /**
     * @brief Get message count
     * @param instance Instance data to read
     * @return Total messages logged
     */
    static uint64_t GetMessageCount(const LoggerInstanceData& instance);

    /**
     * @brief Get error count
     * @param instance Instance data to read
     * @return Total errors logged
     */
    static uint64_t GetErrorCount(const LoggerInstanceData& instance);

    // =================================================================================
    // INSTANCE ACTIVITY TRACKING
    // =================================================================================

    /**
     * @brief Update instance activity timestamp
     * @param instance Instance data to update
     */
    static void UpdateActivity(LoggerInstanceData& instance);

    /**
     * @brief Increment message count
     * @param instance Instance data to update
     */
    static void IncrementMessageCount(LoggerInstanceData& instance);

    /**
     * @brief Increment error count
     * @param instance Instance data to update
     */
    static void IncrementErrorCount(LoggerInstanceData& instance);

    /**
     * @brief Update instance statistics
     * @param instance Instance data to update
     * @param message_count New message count
     * @param error_count New error count
     */
    static void UpdateStatistics(LoggerInstanceData& instance,
                                uint64_t message_count,
                                uint64_t error_count);

    // =================================================================================
    // INSTANCE COMPARISON AND SEARCHING
    // =================================================================================

    /**
     * @brief Compare two instances for equality
     * @param a First instance
     * @param b Second instance
     * @return true if instances are identical
     */
    static bool CompareInstances(const LoggerInstanceData& a, const LoggerInstanceData& b);

    /**
     * @brief Check if two instances represent the same application
     * @param a First instance
     * @param b Second instance
     * @return true if instances are from same application
     */
    static bool AreSameApplication(const LoggerInstanceData& a, const LoggerInstanceData& b);

    /**
     * @brief Check if two instances represent the same process
     * @param a First instance
     * @param b Second instance
     * @return true if instances are from same process
     */
    static bool AreSameProcess(const LoggerInstanceData& a, const LoggerInstanceData& b);

    /**
     * @brief Find instance by ID in a collection
     * @param instances Vector of instances to search
     * @param instance_id ID to search for
     * @return Iterator to found instance or end()
     */
    static std::vector<LoggerInstanceData>::iterator FindInstanceById(
        std::vector<LoggerInstanceData>& instances,
        uint32_t instance_id);

    /**
     * @brief Find instance by application name in a collection
     * @param instances Vector of instances to search
     * @param application_name Application name to search for
     * @return Vector of matching instances
     */
    static std::vector<LoggerInstanceData> FindInstancesByApplication(
        const std::vector<LoggerInstanceData>& instances,
        const std::string& application_name);

    // =================================================================================
    // INSTANCE COLLECTION MANAGEMENT
    // =================================================================================

    /**
     * @brief Filter instances by application name
     * @param instances Vector of instances to filter
     * @param application_name Application name to filter by
     * @return Vector of instances matching the application
     */
    static std::vector<LoggerInstanceData> FilterByApplication(
        const std::vector<LoggerInstanceData>& instances,
        const std::string& application_name);

    /**
     * @brief Filter active instances
     * @param instances Vector of instances to filter
     * @param max_idle_seconds Maximum idle time in seconds
     * @return Vector of active instances
     */
    static std::vector<LoggerInstanceData> FilterActiveInstances(
        const std::vector<LoggerInstanceData>& instances,
        DWORD max_idle_seconds = 300);

    /**
     * @brief Sort instances by creation time
     * @param instances Vector of instances to sort (modified in place)
     */
    static void SortByCreationTime(std::vector<LoggerInstanceData>& instances);

    /**
     * @brief Sort instances by activity
     * @param instances Vector of instances to sort (modified in place)
     */
    static void SortByActivity(std::vector<LoggerInstanceData>& instances);

    /**
     * @brief Remove inactive instances from collection
     * @param instances Vector of instances to clean (modified in place)
     * @param max_idle_seconds Maximum idle time in seconds
     * @return Number of instances removed
     */
    static size_t RemoveInactiveInstances(std::vector<LoggerInstanceData>& instances,
                                         DWORD max_idle_seconds = 300);

    // =================================================================================
    // INSTANCE STATISTICS
    // =================================================================================

    /**
     * @brief Calculate instance statistics
     * @param instance Instance data to analyze
     * @return InstanceStatistics structure
     */
    static InstanceStatistics CalculateStatistics(const LoggerInstanceData& instance);

    /**
     * @brief Calculate uptime in seconds
     * @param instance Instance data to analyze
     * @return Uptime in seconds
     */
    static DWORD CalculateUptime(const LoggerInstanceData& instance);

    /**
     * @brief Calculate message rate (messages per second)
     * @param instance Instance data to analyze
     * @param current_time Current timestamp (optional, defaults to now)
     * @return Message rate as double
     */
    static double CalculateMessageRate(const LoggerInstanceData& instance,
                                      DWORD current_time = 0);

    /**
     * @brief Calculate error rate (errors per second)
     * @param instance Instance data to analyze
     * @param current_time Current timestamp (optional, defaults to now)
     * @return Error rate as double
     */
    static double CalculateErrorRate(const LoggerInstanceData& instance,
                                    DWORD current_time = 0);

    /**
     * @brief Get unique application names from instances
     * @param instances Vector of instances to analyze
     * @return Vector of unique application names
     */
    static std::vector<std::string> GetUniqueApplications(
        const std::vector<LoggerInstanceData>& instances);

    /**
     * @brief Count instances by application
     * @param instances Vector of instances to analyze
     * @return Vector of pairs (application_name, count)
     */
    static std::vector<std::pair<std::string, size_t>> CountByApplication(
        const std::vector<LoggerInstanceData>& instances);

    // =================================================================================
    // INSTANCE FORMATTING
    // =================================================================================

    /**
     * @brief Convert instance to string representation
     * @param instance Instance data to format
     * @return Human-readable string representation
     */
    static std::string InstanceToString(const LoggerInstanceData& instance);

    /**
     * @brief Convert instance to JSON format
     * @param instance Instance data to format
     * @return JSON string representation
     */
    static std::string InstanceToJson(const LoggerInstanceData& instance);

    /**
     * @brief Convert instance to CSV format
     * @param instance Instance data to format
     * @return CSV string representation
     */
    static std::string InstanceToCsv(const LoggerInstanceData& instance);

    /**
     * @brief Format instance for console output
     * @param instance Instance data to format
     * @return Console-formatted string
     */
    static std::string FormatInstanceForConsole(const LoggerInstanceData& instance);

    // =================================================================================
    // SYSTEM INFORMATION GATHERING
    // =================================================================================

    /**
     * @brief Get current process information
     * @return Process ID and name
     */
    static std::pair<DWORD, std::string> GetCurrentProcessInfo();

    /**
     * @brief Get current user name
     * @return Current user name
     */
    static std::string GetCurrentUserName();

    /**
     * @brief Get machine hostname
     * @return Machine hostname
     */
    static std::string GetHostName();

    /**
     * @brief Get current timestamp as DWORD
     * @return Current Unix timestamp
     */
    static DWORD GetCurrentTimestamp();

    /**
     * @brief Generate a human-readable instance name
     * @param application_name Application name
     * @param process_name Process name (optional)
     * @return Generated instance name
     */
    static std::string GenerateInstanceName(const std::string& application_name,
                                           const std::string& process_name = "");

private:
    // Private helper methods
    static bool SetStringField(char* field, const std::string& value, size_t max_length);
    static std::string GetStringField(const char* field);
    static uint64_t CalculateRate(uint64_t count, DWORD uptime_seconds);
    static bool IsStringEmpty(const char* str);
};

#endif // __ASFM_LOGGER_INSTANCE_TOOLBOX_HPP__