#ifndef __ASFM_LOGGER_INSTANCE_HPP__
#define __ASFM_LOGGER_INSTANCE_HPP__

/**
 * ASFMLogger Stateful LoggerInstance
 *
 * Stateful wrapper class for logger instance management using toolbox internally.
 * Provides object-oriented interface for instance lifecycle management.
 */

#include "structs/LoggerInstanceData.hpp"
#include "toolbox/LoggerInstanceToolbox.hpp"
#include <string>
#include <memory>
#include <chrono>
#include <mutex>

class LoggerInstance {
private:
    LoggerInstanceData data_;  // POD data only
    mutable std::mutex instance_mutex_;

public:
    // =================================================================================
    // CONSTRUCTORS AND FACTORY METHODS
    // =================================================================================

    /**
     * @brief Default constructor
     */
    LoggerInstance();

    /**
     * @brief Constructor with application and process names
     * @param application_name Name of the application
     * @param process_name Name of the process (optional)
     * @param instance_name Human-readable instance name (optional)
     */
    LoggerInstance(const std::string& application_name,
                   const std::string& process_name = "",
                   const std::string& instance_name = "");

    /**
     * @brief Constructor from existing LoggerInstanceData
     * @param data Existing instance data
     */
    explicit LoggerInstance(const LoggerInstanceData& data);

    /**
     * @brief Copy constructor
     * @param other Instance to copy from
     */
    LoggerInstance(const LoggerInstance& other);

    /**
     * @brief Move constructor
     * @param other Instance to move from
     */
    LoggerInstance(LoggerInstance&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Instance to assign from
     * @return Reference to this instance
     */
    LoggerInstance& operator=(const LoggerInstance& other);

    /**
     * @brief Move assignment operator
     * @param other Instance to move from
     * @return Reference to this instance
     */
    LoggerInstance& operator=(LoggerInstance&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~LoggerInstance() = default;

    // =================================================================================
    // FACTORY METHODS
    // =================================================================================

    /**
     * @brief Create a new instance using toolbox
     * @param application_name Name of the application
     * @param process_name Name of the process (optional)
     * @param instance_name Human-readable instance name (optional)
     * @return New LoggerInstance instance
     */
    static LoggerInstance Create(const std::string& application_name,
                                const std::string& process_name = "",
                                const std::string& instance_name = "");

    /**
     * @brief Create a copy of this instance
     * @return New LoggerInstance instance
     */
    LoggerInstance clone() const;

    // =================================================================================
    // GETTERS (Using toolbox internally)
    // =================================================================================

    /**
     * @brief Get instance ID
     * @return Instance ID
     */
    uint32_t getId() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetInstanceId(data_);
    }

    /**
     * @brief Get application name
     * @return Application name string
     */
    std::string getApplicationName() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetApplicationName(data_);
    }

    /**
     * @brief Get process name
     * @return Process name string
     */
    std::string getProcessName() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetProcessName(data_);
    }

    /**
     * @brief Get instance name
     * @return Instance name string
     */
    std::string getInstanceName() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetInstanceName(data_);
    }

    /**
     * @brief Get process ID
     * @return Process ID
     */
    DWORD getProcessId() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetProcessId(data_);
    }

    /**
     * @brief Get creation time
     * @return Creation timestamp
     */
    DWORD getCreationTime() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetCreationTime(data_);
    }

    /**
     * @brief Get last activity time
     * @return Last activity timestamp
     */
    DWORD getLastActivity() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetLastActivity(data_);
    }

    /**
     * @brief Get message count
     * @return Total messages logged
     */
    uint64_t getMessageCount() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetMessageCount(data_);
    }

    /**
     * @brief Get error count
     * @return Total errors logged
     */
    uint64_t getErrorCount() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::GetErrorCount(data_);
    }

    /**
     * @brief Get raw data structure (const access)
     * @return Const reference to internal data
     */
    const LoggerInstanceData& getData() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return data_;
    }

    // =================================================================================
    // SETTERS (Using toolbox internally)
    // =================================================================================

    /**
     * @brief Set application information
     * @param application_name Application name
     * @param process_name Process name (optional)
     * @return true if successful
     */
    bool setApplicationInfo(const std::string& application_name, const std::string& process_name = "");

    /**
     * @brief Set instance name
     * @param instance_name Human-readable instance name
     * @return true if successful
     */
    bool setInstanceName(const std::string& instance_name);

    // =================================================================================
    // ACTIVITY TRACKING
    // =================================================================================

    /**
     * @brief Update activity timestamp (thread-safe)
     */
    void updateActivity() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        LoggerInstanceToolbox::UpdateActivity(data_);
    }

    /**
     * @brief Increment message count (thread-safe)
     */
    void incrementMessageCount() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        LoggerInstanceToolbox::IncrementMessageCount(data_);
    }

    /**
     * @brief Increment error count (thread-safe)
     */
    void incrementErrorCount() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        LoggerInstanceToolbox::IncrementErrorCount(data_);
    }

    /**
     * @brief Update statistics (thread-safe)
     * @param message_count New message count
     * @param error_count New error count
     */
    void updateStatistics(uint64_t message_count, uint64_t error_count) {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        LoggerInstanceToolbox::UpdateStatistics(data_, message_count, error_count);
    }

    // =================================================================================
    // VALIDATION AND INSPECTION
    // =================================================================================

    /**
     * @brief Check if instance is valid
     * @return true if instance is valid
     */
    bool isValid() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::ValidateInstance(data_);
    }

    /**
     * @brief Check if instance has application information
     * @return true if application info is set
     */
    bool hasApplicationInfo() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::HasApplicationInfo(data_);
    }

    /**
     * @brief Check if instance is active (recent activity)
     * @param max_idle_seconds Maximum idle time in seconds
     * @return true if instance is considered active
     */
    bool isActive(DWORD max_idle_seconds = 300) const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::IsInstanceActive(data_, max_idle_seconds);
    }

    /**
     * @brief Calculate uptime in seconds
     * @return Uptime in seconds
     */
    DWORD getUptime() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::CalculateUptime(data_);
    }

    /**
     * @brief Calculate message rate (messages per second)
     * @return Message rate as double
     */
    double getMessageRate() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::CalculateMessageRate(data_);
    }

    /**
     * @brief Calculate error rate (errors per second)
     * @return Error rate as double
     */
    double getErrorRate() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::CalculateErrorRate(data_);
    }

    // =================================================================================
    // FORMATTING AND OUTPUT
    // =================================================================================

    /**
     * @brief Convert to string representation
     * @return Human-readable string
     */
    std::string toString() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::InstanceToString(data_);
    }

    /**
     * @brief Convert to JSON format
     * @return JSON string representation
     */
    std::string toJson() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::InstanceToJson(data_);
    }

    /**
     * @brief Convert to CSV format
     * @return CSV string representation
     */
    std::string toCsv() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::InstanceToCsv(data_);
    }

    /**
     * @brief Format for console output
     * @return Console-formatted string
     */
    std::string formatForConsole() const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        return LoggerInstanceToolbox::FormatInstanceForConsole(data_);
    }

    // =================================================================================
    // COMPARISON OPERATORS
    // =================================================================================

    /**
     * @brief Equality operator
     * @param other Instance to compare with
     * @return true if instances are equal
     */
    bool operator==(const LoggerInstance& other) const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        std::lock_guard<std::mutex> other_lock(other.instance_mutex_);
        return LoggerInstanceToolbox::CompareInstances(data_, other.data_);
    }

    /**
     * @brief Inequality operator
     * @param other Instance to compare with
     * @return true if instances are not equal
     */
    bool operator!=(const LoggerInstance& other) const {
        return !(*this == other);
    }

    /**
     * @brief Less than operator (by creation time)
     * @param other Instance to compare with
     * @return true if this instance is older than other
     */
    bool operator<(const LoggerInstance& other) const;

    /**
     * @brief Greater than operator (by creation time)
     * @param other Instance to compare with
     * @return true if this instance is newer than other
     */
    bool operator>(const LoggerInstance& other) const;

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Swap contents with another instance
     * @param other Instance to swap with
     */
    void swap(LoggerInstance& other) noexcept;

    /**
     * @brief Clear instance data
     */
    void clear();

    /**
     * @brief Check if instance is empty
     * @return true if instance has no application info
     */
    bool empty() const {
        return !hasApplicationInfo();
    }

    /**
     * @brief Get time since last activity
     * @return Seconds since last activity
     */
    DWORD getIdleTime() const;

    /**
     * @brief Check if instance belongs to same application as another
     * @param other Other instance to compare with
     * @return true if both instances are from same application
     */
    bool isSameApplication(const LoggerInstance& other) const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        std::lock_guard<std::mutex> other_lock(other.instance_mutex_);
        return LoggerInstanceToolbox::AreSameApplication(data_, other.data_);
    }

    /**
     * @brief Check if instance belongs to same process as another
     * @param other Other instance to compare with
     * @return true if both instances are from same process
     */
    bool isSameProcess(const LoggerInstance& other) const {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        std::lock_guard<std::mutex> other_lock(other.instance_mutex_);
        return LoggerInstanceToolbox::AreSameProcess(data_, other.data_);
    }
};

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

/**
 * @brief Swap two LoggerInstance objects
 * @param a First instance
 * @param b Second instance
 */
void swap(LoggerInstance& a, LoggerInstance& b) noexcept;

/**
 * @brief Output stream operator for LoggerInstance
 * @param os Output stream
 * @param instance Instance to output
 * @return Reference to output stream
 */
std::ostream& operator<<(std::ostream& os, const LoggerInstance& instance);

/**
 * @brief Create a LoggerInstance for current application
 * @param application_name Name of the application
 * @param process_name Name of the process (optional)
 * @return New LoggerInstance for current application
 */
LoggerInstance CreateCurrentApplicationInstance(const std::string& application_name,
                                               const std::string& process_name = "");

#endif // __ASFM_LOGGER_INSTANCE_HPP__