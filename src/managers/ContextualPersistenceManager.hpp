#ifndef __ASFM_LOGGER_CONTEXTUAL_PERSISTENCE_MANAGER_HPP__
#define __ASFM_LOGGER_CONTEXTUAL_PERSISTENCE_MANAGER_HPP__

/**
 * ASFMLogger Contextual Persistence Manager
 *
 * Stateful manager class for contextual persistence policy management.
 * Uses ContextualPersistenceToolbox internally for all decision logic.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/PersistencePolicy.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include "toolbox/ContextualPersistenceToolbox.hpp"
#include "stateful/ImportanceMapper.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>

class ContextualPersistenceManager {
private:
    std::string application_name_;
    PersistencePolicy current_policy_;
    ImportanceMapper* importance_mapper_;
    mutable std::mutex manager_mutex_;

    // Emergency mode tracking
    bool emergency_mode_active_;
    DWORD emergency_mode_end_time_;
    std::chrono::steady_clock::time_point last_policy_evaluation_;

    // Statistics tracking
    uint64_t total_decisions_made_;
    uint64_t total_messages_persisted_;
    uint64_t total_messages_dropped_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    ContextualPersistenceManager();

    /**
     * @brief Constructor with application name
     * @param application_name Name of the application this manager serves
     */
    explicit ContextualPersistenceManager(const std::string& application_name);

    /**
     * @brief Constructor with application name and policy
     * @param application_name Name of the application
     * @param policy Initial persistence policy
     */
    ContextualPersistenceManager(const std::string& application_name, const PersistencePolicy& policy);

    /**
     * @brief Constructor with importance mapper
     * @param application_name Name of the application
     * @param importance_mapper Importance mapper to use for decisions
     */
    ContextualPersistenceManager(const std::string& application_name, ImportanceMapper* importance_mapper);

    /**
     * @brief Copy constructor
     * @param other Manager to copy from
     */
    ContextualPersistenceManager(const ContextualPersistenceManager& other);

    /**
     * @brief Move constructor
     * @param other Manager to move from
     */
    ContextualPersistenceManager(ContextualPersistenceManager&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Manager to assign from
     * @return Reference to this manager
     */
    ContextualPersistenceManager& operator=(const ContextualPersistenceManager& other);

    /**
     * @brief Move assignment operator
     * @param other Manager to move from
     * @return Reference to this manager
     */
    ContextualPersistenceManager& operator=(ContextualPersistenceManager&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~ContextualPersistenceManager();

    // =================================================================================
    // APPLICATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Set application name
     * @param application_name Name of the application
     */
    void setApplicationName(const std::string& application_name);

    /**
     * @brief Get application name
     * @return Application name
     */
    std::string getApplicationName() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return application_name_;
    }

    /**
     * @brief Set importance mapper for decision making
     * @param importance_mapper Importance mapper to use
     */
    void setImportanceMapper(ImportanceMapper* importance_mapper);

    /**
     * @brief Get importance mapper
     * @return Pointer to importance mapper or nullptr if not set
     */
    ImportanceMapper* getImportanceMapper() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return importance_mapper_;
    }

    // =================================================================================
    // POLICY MANAGEMENT
    // =================================================================================

    /**
     * @brief Set persistence policy
     * @param policy New persistence policy
     * @return true if policy is valid and was set
     */
    bool setPolicy(const PersistencePolicy& policy);

    /**
     * @brief Get current persistence policy
     * @return Current policy
     */
    PersistencePolicy getPolicy() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return current_policy_;
    }

    /**
     * @brief Load policy from configuration
     * @param config Policy configuration to load
     * @return true if policy is valid and was loaded
     */
    bool loadPolicy(const PersistencePolicy& config);

    /**
     * @brief Reset policy to defaults
     * @return true if reset successful
     */
    bool resetToDefaults();

    /**
     * @brief Create high-performance policy
     * @return true if policy was created successfully
     */
    bool createHighPerformancePolicy();

    /**
     * @brief Create comprehensive policy for detailed logging
     * @return true if policy was created successfully
     */
    bool createComprehensivePolicy();

    /**
     * @brief Create balanced policy for general use
     * @return true if policy was created successfully
     */
    bool createBalancedPolicy();

    // =================================================================================
    // PERSISTENCE DECISIONS
    // =================================================================================

    /**
     * @brief Make persistence decision for a single message
     * @param message Message to evaluate
     * @return Detailed decision result with reasoning
     */
    PersistenceDecisionResult makePersistenceDecision(const LogMessageData& message);

    /**
     * @brief Quick persistence check for high-performance scenarios
     * @param message_type Type of the message
     * @param resolved_importance Importance of the message
     * @return true if message should be persisted
     */
    bool shouldPersistQuick(LogMessageType message_type, MessageImportance resolved_importance);

    /**
     * @brief Make persistence decisions for multiple messages
     * @param messages Vector of messages to evaluate
     * @return Vector of decision results
     */
    std::vector<PersistenceDecisionResult> makeBatchPersistenceDecisions(
        const std::vector<LogMessageData>& messages);

    /**
     * @brief Filter messages that should be persisted
     * @param messages Vector of messages to filter
     * @return Vector of messages that should be persisted
     */
    std::vector<LogMessageData> filterPersistableMessages(const std::vector<LogMessageData>& messages);

    // =================================================================================
    // ADAPTIVE POLICY MANAGEMENT
    // =================================================================================

    /**
     * @brief Evaluate and adapt policy based on current conditions
     * @return true if policy was adapted
     */
    bool evaluateAndAdaptPolicy();

    /**
     * @brief Force policy evaluation regardless of timing
     * @return true if policy was adapted
     */
    bool forcePolicyEvaluation();

    /**
     * @brief Set adaptive policy trigger
     * @param trigger Trigger configuration
     * @return true if trigger was set successfully
     */
    bool setAdaptiveTrigger(const AdaptivePolicyTrigger& trigger);

    /**
     * @brief Remove adaptive policy trigger
     * @param trigger_id Trigger ID to remove
     * @return true if trigger was found and removed
     */
    bool removeAdaptiveTrigger(uint32_t trigger_id);

    /**
     * @brief Get all adaptive triggers for this application
     * @return Vector of adaptive triggers
     */
    std::vector<AdaptivePolicyTrigger> getAdaptiveTriggers() const;

    // =================================================================================
    // EMERGENCY MODE MANAGEMENT
    // =================================================================================

    /**
     * @brief Enter emergency persistence mode
     * @param duration_seconds How long emergency mode should last
     * @return true if emergency mode was activated
     */
    bool enterEmergencyMode(DWORD duration_seconds = 300);

    /**
     * @brief Exit emergency persistence mode
     * @return true if emergency mode was deactivated
     */
    bool exitEmergencyMode();

    /**
     * @brief Check if application is in emergency mode
     * @return true if application is in emergency mode
     */
    bool isInEmergencyMode() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return emergency_mode_active_ && GetCurrentTimestamp() < emergency_mode_end_time_;
    }

    /**
     * @brief Get time remaining in emergency mode
     * @return Seconds remaining in emergency mode, 0 if not in emergency mode
     */
    DWORD getEmergencyModeTimeRemaining() const;

    /**
     * @brief Extend emergency mode
     * @param additional_seconds Additional seconds to extend
     * @return true if emergency mode was extended
     */
    bool extendEmergencyMode(DWORD additional_seconds);

    // =================================================================================
    // STATISTICS AND MONITORING
    // =================================================================================

    /**
     * @brief Get persistence statistics
     * @return Current persistence statistics
     */
    PersistenceStatistics getStatistics() const;

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Get decision statistics
     * @return String containing decision statistics
     */
    std::string getDecisionStatistics() const;

    /**
     * @brief Get policy effectiveness metrics
     * @return Vector of effectiveness metrics
     */
    std::vector<std::pair<std::string, double>> getEffectivenessMetrics() const;

    /**
     * @brief Update statistics after persistence operation
     * @param decision Decision that was made
     * @param actual_persistence_time_ms Actual time taken to persist
     */
    void updateStatistics(const PersistenceDecisionResult& decision, DWORD actual_persistence_time_ms);

    /**
     * @brief Get total decisions made
     * @return Total number of persistence decisions
     */
    uint64_t getTotalDecisions() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return total_decisions_made_;
    }

    /**
     * @brief Get total messages persisted
     * @return Total number of persisted messages
     */
    uint64_t getTotalPersisted() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return total_messages_persisted_;
    }

    /**
     * @brief Get total messages dropped
     * @return Total number of dropped messages
     */
    uint64_t getTotalDropped() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return total_messages_dropped_;
    }

    // =================================================================================
    // CONFIGURATION PERSISTENCE
    // =================================================================================

    /**
     * @brief Save current policy to file
     * @param config_file Path to save configuration file
     * @return true if configuration saved successfully
     */
    bool savePolicyToFile(const std::string& config_file) const;

    /**
     * @brief Load policy from file
     * @param config_file Path to configuration file
     * @return true if configuration loaded successfully
     */
    bool loadPolicyFromFile(const std::string& config_file);

    /**
     * @brief Export current policy as string
     * @return Policy as formatted string
     */
    std::string exportPolicy() const;

    /**
     * @brief Import policy from string
     * @param policy_string Policy as formatted string
     * @return true if policy imported successfully
     */
    bool importPolicy(const std::string& policy_string);

    // =================================================================================
    // CONTEXT EVALUATION
    // =================================================================================

    /**
     * @brief Evaluate current system context
     * @return Current system context for decision making
     */
    PersistenceDecisionContext evaluateCurrentContext() const;

    /**
     * @brief Check if system is under high load
     * @return true if system is under high load
     */
    bool isHighLoad() const;

    /**
     * @brief Check if error rate is elevated
     * @return true if error rate is elevated
     */
    bool isElevatedErrorRate() const;

    /**
     * @brief Check if emergency conditions are present
     * @return true if emergency conditions exist
     */
    bool isEmergencyCondition() const;

    /**
     * @brief Get current system stress level (0-100)
     * @return Stress level as percentage
     */
    DWORD getSystemStressLevel() const;

    // =================================================================================
    // COMPONENT AND CRITICAL PATH MANAGEMENT
    // =================================================================================

    /**
     * @brief Add critical component that should always be persisted
     * @param component_pattern Component name pattern
     * @return true if component was added successfully
     */
    bool addCriticalComponent(const std::string& component_pattern);

    /**
     * @brief Remove critical component
     * @param component_pattern Component name pattern to remove
     * @return true if component was found and removed
     */
    bool removeCriticalComponent(const std::string& component_pattern);

    /**
     * @brief Add ignored component that should never be persisted
     * @param component_pattern Component name pattern
     * @return true if component was added successfully
     */
    bool addIgnoredComponent(const std::string& component_pattern);

    /**
     * @brief Remove ignored component
     * @param component_pattern Component name pattern to remove
     * @return true if component was found and removed
     */
    bool removeIgnoredComponent(const std::string& component_pattern);

    /**
     * @brief Get all critical components
     * @return Vector of critical component patterns
     */
    std::vector<std::string> getCriticalComponents() const;

    /**
     * @brief Get all ignored components
     * @return Vector of ignored component patterns
     */
    std::vector<std::string> getIgnoredComponents() const;

    /**
     * @brief Check if component is critical
     * @param component Component name to check
     * @return true if component is marked critical
     */
    bool isComponentCritical(const std::string& component) const;

    /**
     * @brief Check if component is ignored
     * @param component Component name to check
     * @return true if component is marked ignored
     */
    bool isComponentIgnored(const std::string& component) const;

    // =================================================================================
    // PERFORMANCE OPTIMIZATION
    // =================================================================================

    /**
     * @brief Optimize policy for current conditions
     * @return true if policy was optimized
     */
    bool optimizeForCurrentConditions();

    /**
     * @brief Set performance mode
     * @param high_performance Whether to optimize for high performance
     * @return true if mode was set successfully
     */
    bool setPerformanceMode(bool high_performance);

    /**
     * @brief Get recommended batch size for current conditions
     * @return Recommended batch size
     */
    size_t getRecommendedBatchSize() const;

    /**
     * @brief Check if batch persistence should be used
     * @param message_count Number of messages to evaluate
     * @return true if batch persistence is recommended
     */
    bool shouldUseBatchPersistence(size_t message_count) const;

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Validate current configuration
     * @return true if configuration is valid
     */
    bool validateConfiguration() const;

    /**
     * @brief Clone this manager configuration
     * @return New ContextualPersistenceManager with same configuration
     */
    ContextualPersistenceManager clone() const;

    /**
     * @brief Swap contents with another manager
     * @param other Manager to swap with
     */
    void swap(ContextualPersistenceManager& other) noexcept;

    /**
     * @brief Clear all settings and reset to defaults
     */
    void clear();

    /**
     * @brief Check if manager is properly configured
     * @return true if manager has valid configuration
     */
    bool isConfigured() const;

    /**
     * @brief Get manager status as formatted string
     * @return Status information
     */
    std::string getStatus() const;

private:
    // Private helper methods
    void initializeWithDefaults();
    void updateInternalStatistics(const PersistenceDecisionResult& decision, DWORD persistence_time_ms);
    PersistenceDecisionContext createCurrentContext() const;
    bool shouldEvaluatePolicy() const;
    DWORD GetCurrentTimestamp() const;
};

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

/**
 * @brief Get global persistence manager for an application
 * @param application_name Name of the application
 * @return Reference to global ContextualPersistenceManager
 */
ContextualPersistenceManager& GetGlobalPersistenceManager(const std::string& application_name);

/**
 * @brief Create persistence manager for current application
 * @param application_name Name of the application
 * @return New ContextualPersistenceManager for current application
 */
ContextualPersistenceManager CreateApplicationPersistenceManager(const std::string& application_name);

#endif // __ASFM_LOGGER_CONTEXTUAL_PERSISTENCE_MANAGER_HPP__