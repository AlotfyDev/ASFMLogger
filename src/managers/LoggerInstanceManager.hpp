#ifndef __ASFM_LOGGER_INSTANCE_MANAGER_HPP__
#define __ASFM_LOGGER_INSTANCE_MANAGER_HPP__

/**
 * ASFMLogger Instance Manager
 *
 * Stateful manager class for coordinating multiple logger instances.
 * Uses LoggerInstanceToolbox internally for all operations.
 */

#include "structs/LoggerInstanceData.hpp"
#include "stateful/LoggerInstance.hpp"
#include "toolbox/LoggerInstanceToolbox.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>
#include <chrono>

class LoggerInstanceManager {
private:
    std::vector<LoggerInstanceData> instances_;
    std::unordered_map<uint32_t, size_t> instance_index_map_;  // instance_id -> vector index
    std::unordered_map<std::string, std::vector<uint32_t>> application_instances_;
    mutable std::mutex manager_mutex_;

    // Cleanup configuration
    DWORD cleanup_interval_seconds_;
    DWORD max_idle_time_seconds_;
    std::chrono::steady_clock::time_point last_cleanup_;

public:
    // =================================================================================
    // CONSTRUCTORS AND SINGLETON
    // =================================================================================

    /**
     * @brief Default constructor
     */
    LoggerInstanceManager();

    /**
     * @brief Constructor with cleanup configuration
     * @param cleanup_interval_seconds Interval between cleanup operations
     * @param max_idle_time_seconds Maximum idle time before instance cleanup
     */
    LoggerInstanceManager(DWORD cleanup_interval_seconds, DWORD max_idle_time_seconds);

    /**
     * @brief Destructor
     */
    ~LoggerInstanceManager();

    // =================================================================================
    // INSTANCE REGISTRATION
    // =================================================================================

    /**
     * @brief Register a new logger instance
     * @param application_name Name of the application
     * @param process_name Name of the process (optional)
     * @param instance_name Human-readable instance name (optional)
     * @return New LoggerInstance object
     */
    LoggerInstance registerInstance(const std::string& application_name,
                                   const std::string& process_name = "",
                                   const std::string& instance_name = "");

    /**
     * @brief Register instance from existing data
     * @param instance_data Existing instance data
     * @return LoggerInstance object wrapping the data
     */
    LoggerInstance registerInstance(const LoggerInstanceData& instance_data);

    /**
     * @brief Register instance from LoggerInstance object
     * @param instance Existing LoggerInstance object
     * @return true if registration successful
     */
    bool registerInstance(const LoggerInstance& instance);

    // =================================================================================
    // INSTANCE LOOKUP
    // =================================================================================

    /**
     * @brief Find instance by ID
     * @param instance_id Instance ID to search for
     * @return LoggerInstance object or invalid instance if not found
     */
    LoggerInstance findInstance(uint32_t instance_id) const;

    /**
     * @brief Find instance by application name
     * @param application_name Application name to search for
     * @return Vector of matching LoggerInstance objects
     */
    std::vector<LoggerInstance> findInstancesByApplication(const std::string& application_name) const;

    /**
     * @brief Find instance by process name
     * @param process_name Process name to search for
     * @return Vector of matching LoggerInstance objects
     */
    std::vector<LoggerInstance> findInstancesByProcess(const std::string& process_name) const;

    /**
     * @brief Get all registered instances
     * @return Vector of all LoggerInstance objects
     */
    std::vector<LoggerInstance> getAllInstances() const;

    /**
     * @brief Get active instances only
     * @return Vector of active LoggerInstance objects
     */
    std::vector<LoggerInstance> getActiveInstances() const;

    // =================================================================================
    // INSTANCE MANAGEMENT
    // =================================================================================

    /**
     * @brief Update instance activity
     * @param instance_id Instance ID to update
     * @return true if instance found and updated
     */
    bool updateInstanceActivity(uint32_t instance_id);

    /**
     * @brief Update instance statistics
     * @param instance_id Instance ID to update
     * @param message_count New message count
     * @param error_count New error count
     * @return true if instance found and updated
     */
    bool updateInstanceStatistics(uint32_t instance_id, uint64_t message_count, uint64_t error_count);

    /**
     * @brief Increment message count for instance
     * @param instance_id Instance ID to update
     * @return true if instance found and updated
     */
    bool incrementMessageCount(uint32_t instance_id);

    /**
     * @brief Increment error count for instance
     * @param instance_id Instance ID to update
     * @return true if instance found and updated
     */
    bool incrementErrorCount(uint32_t instance_id);

    /**
     * @brief Unregister instance
     * @param instance_id Instance ID to unregister
     * @return true if instance was found and removed
     */
    bool unregisterInstance(uint32_t instance_id);

    /**
     * @brief Unregister all instances for an application
     * @param application_name Application name
     * @return Number of instances removed
     */
    size_t unregisterApplicationInstances(const std::string& application_name);

    // =================================================================================
    // CLEANUP AND MAINTENANCE
    // =================================================================================

    /**
     * @brief Clean up inactive instances
     * @return Number of instances removed
     */
    size_t cleanupInactiveInstances();

    /**
     * @brief Force cleanup regardless of timing
     * @return Number of instances removed
     */
    size_t forceCleanup();

    /**
     * @brief Set cleanup configuration
     * @param cleanup_interval_seconds Interval between cleanup operations
     * @param max_idle_time_seconds Maximum idle time before cleanup
     */
    void setCleanupConfiguration(DWORD cleanup_interval_seconds, DWORD max_idle_time_seconds);

    /**
     * @brief Get cleanup configuration
     * @param cleanup_interval_seconds Output cleanup interval
     * @param max_idle_time_seconds Output max idle time
     */
    void getCleanupConfiguration(DWORD& cleanup_interval_seconds, DWORD& max_idle_time_seconds) const;

    // =================================================================================
    // STATISTICS AND MONITORING
    // =================================================================================

    /**
     * @brief Get total number of registered instances
     * @return Total instance count
     */
    size_t getInstanceCount() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return instances_.size();
    }

    /**
     * @brief Get number of active instances
     * @return Active instance count
     */
    size_t getActiveInstanceCount() const;

    /**
     * @brief Get unique application names
     * @return Vector of unique application names
     */
    std::vector<std::string> getUniqueApplications() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return LoggerInstanceToolbox::GetUniqueApplications(instances_);
    }

    /**
     * @brief Get instance count by application
     * @return Vector of pairs (application_name, count)
     */
    std::vector<std::pair<std::string, size_t>> getInstanceCountByApplication() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return LoggerInstanceToolbox::CountByApplication(instances_);
    }

    /**
     * @brief Get total message count across all instances
     * @return Total messages logged
     */
    uint64_t getTotalMessageCount() const;

    /**
     * @brief Get total error count across all instances
     * @return Total errors logged
     */
    uint64_t getTotalErrorCount() const;

    /**
     * @brief Get overall message rate (messages per second)
     * @return Overall message rate
     */
    double getOverallMessageRate() const;

    /**
     * @brief Get overall error rate (errors per second)
     * @return Overall error rate
     */
    double getOverallErrorRate() const;

    // =================================================================================
    // INSTANCE VALIDATION
    // =================================================================================

    /**
     * @brief Check if instance is registered
     * @param instance_id Instance ID to check
     * @return true if instance is registered
     */
    bool isInstanceRegistered(uint32_t instance_id) const;

    /**
     * @brief Check if application has registered instances
     * @param application_name Application name to check
     * @return true if application has registered instances
     */
    bool hasApplicationInstances(const std::string& application_name) const;

    /**
     * @brief Validate all registered instances
     * @return Vector of validation results (instance_id, is_valid)
     */
    std::vector<std::pair<uint32_t, bool>> validateAllInstances() const;

    // =================================================================================
    // BULK OPERATIONS
    // =================================================================================

    /**
     * @brief Update activity for all instances of an application
     * @param application_name Application name
     * @return Number of instances updated
     */
    size_t updateApplicationActivity(const std::string& application_name);

    /**
     * @brief Get instances sorted by activity
     * @return Vector of instances sorted by last activity (newest first)
     */
    std::vector<LoggerInstance> getInstancesSortedByActivity() const;

    /**
     * @brief Get instances sorted by message count
     * @return Vector of instances sorted by message count (highest first)
     */
    std::vector<LoggerInstance> getInstancesSortedByMessageCount() const;

    /**
     * @brief Export all instances to vector for external processing
     * @return Vector of all instance data
     */
    std::vector<LoggerInstanceData> exportAllInstances() const;

    /**
     * @brief Clear all instances (use with caution)
     * @return Number of instances removed
     */
    size_t clearAllInstances();

    // =================================================================================
    // MONITORING AND DIAGNOSTICS
    // =================================================================================

    /**
     * @brief Get manager statistics
     * @return String containing manager statistics
     */
    std::string getManagerStatistics() const;

    /**
     * @brief Check if cleanup is needed
     * @return true if cleanup should be performed
     */
    bool isCleanupNeeded() const;

    /**
     * @brief Get time since last cleanup
     * @return Seconds since last cleanup
     */
    DWORD getTimeSinceLastCleanup() const;

    /**
     * @brief Force refresh of internal indexes
     * @return true if refresh successful
     */
    bool refreshIndexes();

private:
    // Private helper methods
    void performCleanupIfNeeded();
    void rebuildIndexes();
    void addInstanceToIndexes(const LoggerInstanceData& instance);
    void removeInstanceFromIndexes(uint32_t instance_id);
    size_t findInstanceIndex(uint32_t instance_id) const;
    std::vector<uint32_t> getApplicationInstanceIds(const std::string& application_name) const;
};

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

/**
 * @brief Get the global instance manager (singleton pattern)
 * @return Reference to global LoggerInstanceManager
 */
LoggerInstanceManager& GetGlobalInstanceManager();

/**
 * @brief Register current application instance globally
 * @param application_name Name of the application
 * @param process_name Name of the process (optional)
 * @return LoggerInstance for current application
 */
LoggerInstance RegisterCurrentApplication(const std::string& application_name,
                                         const std::string& process_name = "");

#endif // __ASFM_LOGGER_INSTANCE_MANAGER_HPP__