#ifndef __ASFM_LOGGER_CONTEXTUAL_PERSISTENCE_TOOLBOX_HPP__
#define __ASFM_LOGGER_CONTEXTUAL_PERSISTENCE_TOOLBOX_HPP__

/**
 * ASFMLogger Contextual Persistence Toolbox
 *
 * Static methods for contextual persistence decisions following toolbox architecture.
 * Pure functions for smart storage decision-making based on system conditions.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/PersistencePolicy.hpp"
#include "structs/ImportanceConfiguration.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations
struct LogMessageData;
struct PersistencePolicy;
struct PersistenceDecisionContext;
struct PersistenceDecisionResult;
struct PersistenceStatistics;
struct AdaptivePolicyTrigger;

class ContextualPersistenceToolbox {
private:
    // Only static variables allowed in toolbox
    static std::unordered_map<std::string, PersistencePolicy> application_policies_;
    static std::unordered_map<std::string, AdaptivePolicyTrigger> adaptive_triggers_;
    static std::unordered_map<std::string, PersistenceStatistics> persistence_stats_;

public:
    // =================================================================================
    // PERSISTENCE DECISION MAKING (CORE LOGIC)
    // =================================================================================

    /**
     * @brief Make persistence decision for a single message
     * @param message Message to evaluate
     * @param context Runtime context for decision making
     * @param policy Policy to apply for the decision
     * @return Detailed decision result with reasoning
     */
    static PersistenceDecisionResult MakePersistenceDecision(
        const LogMessageData& message,
        const PersistenceDecisionContext& context,
        const PersistencePolicy& policy);

    /**
     * @brief Quick persistence check for high-performance scenarios
     * @param message_type Type of the message
     * @param resolved_importance Importance of the message
     * @param system_load Current system load (0-100)
     * @param policy Policy to apply
     * @return true if message should be persisted
     */
    static bool ShouldPersistQuick(LogMessageType message_type,
                                  MessageImportance resolved_importance,
                                  DWORD system_load,
                                  const PersistencePolicy& policy);

    /**
     * @brief Evaluate persistence based on component rules
     * @param component Component name
     * @param message_type Message type
     * @param policy Policy to apply
     * @return true if component should be persisted
     */
    static bool ShouldPersistByComponent(const std::string& component,
                                        LogMessageType message_type,
                                        const PersistencePolicy& policy);

    /**
     * @brief Evaluate persistence based on system conditions
     * @param message_type Message type
     * @param resolved_importance Resolved importance level
     * @param context System context
     * @param policy Policy to apply
     * @return true if system conditions warrant persistence
     */
    static bool ShouldPersistBySystemConditions(LogMessageType message_type,
                                               MessageImportance resolved_importance,
                                               const PersistenceDecisionContext& context,
                                               const PersistencePolicy& policy);

    /**
     * @brief Check if message should be persisted in emergency mode
     * @param message_type Message type
     * @param context System context
     * @param policy Policy to apply
     * @return true if emergency persistence is warranted
     */
    static bool ShouldPersistInEmergencyMode(LogMessageType message_type,
                                            const PersistenceDecisionContext& context,
                                            const PersistencePolicy& policy);

    // =================================================================================
    // POLICY MANAGEMENT
    // =================================================================================

    /**
     * @brief Set persistence policy for an application
     * @param application_name Application name
     * @param policy Policy to apply
     * @return true if policy was set successfully
     */
    static bool SetApplicationPolicy(const std::string& application_name,
                                    const PersistencePolicy& policy);

    /**
     * @brief Get persistence policy for an application
     * @param application_name Application name
     * @return Policy for the application or default policy if not found
     */
    static PersistencePolicy GetApplicationPolicy(const std::string& application_name);

    /**
     * @brief Create default persistence policy for an application
     * @param application_name Application name
     * @return Default policy configured for the application
     */
    static PersistencePolicy CreateDefaultPolicy(const std::string& application_name);

    /**
     * @brief Create high-performance persistence policy
     * @param application_name Application name
     * @return Policy optimized for high-performance scenarios
     */
    static PersistencePolicy CreateHighPerformancePolicy(const std::string& application_name);

    /**
     * @brief Create comprehensive persistence policy for detailed logging
     * @param application_name Application name
     * @return Policy optimized for comprehensive logging scenarios
     */
    static PersistencePolicy CreateComprehensivePolicy(const std::string& application_name);

    /**
     * @brief Validate persistence policy
     * @param policy Policy to validate
     * @return true if policy is valid
     */
    static bool ValidatePolicy(const PersistencePolicy& policy);

    // =================================================================================
    // ADAPTIVE POLICY MANAGEMENT
    // =================================================================================

    /**
     * @brief Add adaptive policy trigger
     * @param application_name Application name
     * @param trigger Trigger configuration
     * @return true if trigger was added successfully
     */
    static bool AddAdaptiveTrigger(const std::string& application_name,
                                  const AdaptivePolicyTrigger& trigger);

    /**
     * @brief Remove adaptive policy trigger
     * @param application_name Application name
     * @param trigger_id Trigger ID to remove
     * @return true if trigger was found and removed
     */
    static bool RemoveAdaptiveTrigger(const std::string& application_name, uint32_t trigger_id);

    /**
     * @brief Check if any adaptive triggers should fire
     * @param application_name Application name
     * @param context Current system context
     * @return Vector of triggered policy modifications
     */
    static std::vector<PersistencePolicy> CheckAdaptiveTriggers(
        const std::string& application_name,
        const PersistenceDecisionContext& context);

    /**
     * @brief Update policy based on current error rate
     * @param application_name Application name
     * @param current_error_rate Current error rate
     * @return Updated policy if adaptation occurred, null otherwise
     */
    static PersistencePolicy* AdaptPolicyForErrorRate(const std::string& application_name,
                                                     DWORD current_error_rate);

    /**
     * @brief Update policy based on current system load
     * @param application_name Application name
     * @param current_load Current system load (0-100)
     * @return Updated policy if adaptation occurred, null otherwise
     */
    static PersistencePolicy* AdaptPolicyForSystemLoad(const std::string& application_name,
                                                       DWORD current_load);

    // =================================================================================
    // EMERGENCY MODE MANAGEMENT
    // =================================================================================

    /**
     * @brief Enter emergency persistence mode
     * @param application_name Application name
     * @param duration_seconds How long emergency mode should last
     * @return true if emergency mode was activated
     */
    static bool EnterEmergencyMode(const std::string& application_name, DWORD duration_seconds = 300);

    /**
     * @brief Exit emergency persistence mode
     * @param application_name Application name
     * @return true if emergency mode was deactivated
     */
    static bool ExitEmergencyMode(const std::string& application_name);

    /**
     * @brief Check if application is in emergency mode
     * @param application_name Application name
     * @return true if application is in emergency mode
     */
    static bool IsInEmergencyMode(const std::string& application_name);

    /**
     * @brief Get time remaining in emergency mode
     * @param application_name Application name
     * @return Seconds remaining in emergency mode, 0 if not in emergency mode
     */
    static DWORD GetEmergencyModeTimeRemaining(const std::string& application_name);

    // =================================================================================
    // BATCH PERSISTENCE DECISIONS
    // =================================================================================

    /**
     * @brief Make persistence decisions for multiple messages
     * @param messages Vector of messages to evaluate
     * @param context Runtime context
     * @param policy Policy to apply
     * @return Vector of decision results
     */
    static std::vector<PersistenceDecisionResult> MakeBatchPersistenceDecisions(
        const std::vector<LogMessageData>& messages,
        const PersistenceDecisionContext& context,
        const PersistencePolicy& policy);

    /**
     * @brief Filter messages that should be persisted
     * @param messages Vector of messages to filter
     * @param context Runtime context
     * @param policy Policy to apply
     * @return Vector of messages that should be persisted
     */
    static std::vector<LogMessageData> FilterPersistableMessages(
        const std::vector<LogMessageData>& messages,
        const PersistenceDecisionContext& context,
        const PersistencePolicy& policy);

    /**
     * @brief Group messages by persistence method
     * @param messages Vector of messages to group
     * @param decisions Vector of corresponding decisions
     * @return Map of persistence method to message vectors
     */
    static std::unordered_map<std::string, std::vector<LogMessageData>> GroupByPersistenceMethod(
        const std::vector<LogMessageData>& messages,
        const std::vector<PersistenceDecisionResult>& decisions);

    // =================================================================================
    // PERFORMANCE OPTIMIZATION
    // =================================================================================

    /**
     * @brief Optimize policy for current system conditions
     * @param policy Policy to optimize
     * @param context Current system context
     * @return Optimized policy
     */
    static PersistencePolicy OptimizePolicyForConditions(const PersistencePolicy& policy,
                                                        const PersistenceDecisionContext& context);

    /**
     * @brief Calculate optimal batch size for current conditions
     * @param context Current system context
     * @param base_batch_size Base batch size configuration
     * @return Optimal batch size for current conditions
     */
    static size_t CalculateOptimalBatchSize(const PersistenceDecisionContext& context,
                                           size_t base_batch_size);

    /**
     * @brief Determine if batch persistence should be used
     * @param message_count Number of messages to persist
     * @param context Current system context
     * @return true if batch persistence is recommended
     */
    static bool ShouldUseBatchPersistence(size_t message_count,
                                         const PersistenceDecisionContext& context);

    // =================================================================================
    // STATISTICS AND MONITORING
    // =================================================================================

    /**
     * @brief Update persistence statistics
     * @param application_name Application name
     * @param decision Decision that was made
     * @param actual_persistence_time_ms Actual time taken to persist
     */
    static void UpdatePersistenceStatistics(const std::string& application_name,
                                           const PersistenceDecisionResult& decision,
                                           DWORD actual_persistence_time_ms);

    /**
     * @brief Get persistence statistics for an application
     * @param application_name Application name
     * @return Persistence statistics or empty stats if not found
     */
    static PersistenceStatistics GetPersistenceStatistics(const std::string& application_name);

    /**
     * @brief Reset persistence statistics for an application
     * @param application_name Application name
     */
    static void ResetPersistenceStatistics(const std::string& application_name);

    /**
     * @brief Analyze persistence effectiveness
     * @param application_name Application name
     * @return Analysis results as formatted string
     */
    static std::string AnalyzePersistenceEffectiveness(const std::string& application_name);

    /**
     * @brief Get persistence efficiency metrics
     * @param application_name Application name
     * @return Vector of efficiency metrics
     */
    static std::vector<std::pair<std::string, double>> GetPersistenceEfficiencyMetrics(
        const std::string& application_name);

    // =================================================================================
    // CONTEXT EVALUATION
    // =================================================================================

    /**
     * @brief Evaluate current system conditions
     * @param application_name Application name
     * @return Current system context
     */
    static PersistenceDecisionContext EvaluateSystemContext(const std::string& application_name);

    /**
     * @brief Check if system is under high load
     * @param context System context to evaluate
     * @return true if system is under high load
     */
    static bool IsHighLoad(const PersistenceDecisionContext& context);

    /**
     * @brief Check if error rate is elevated
     * @param context System context to evaluate
     * @return true if error rate is elevated
     */
    static bool IsElevatedErrorRate(const PersistenceDecisionContext& context);

    /**
     * @brief Check if emergency conditions are present
     * @param context System context to evaluate
     * @return true if emergency conditions exist
     */
    static bool IsEmergencyCondition(const PersistenceDecisionContext& context);

    /**
     * @brief Calculate system stress level (0-100)
     * @param context System context to evaluate
     * @return Stress level as percentage
     */
    static DWORD CalculateSystemStressLevel(const PersistenceDecisionContext& context);

    // =================================================================================
    // CONFIGURATION AND DEFAULTS
    // =================================================================================

    /**
     * @brief Load persistence policies from configuration file
     * @param config_file Path to configuration file
     * @return true if configuration loaded successfully
     */
    static bool LoadPoliciesFromFile(const std::string& config_file);

    /**
     * @brief Save persistence policies to configuration file
     * @param config_file Path to save configuration file
     * @return true if configuration saved successfully
     */
    static bool SavePoliciesToFile(const std::string& config_file);

    /**
     * @brief Reset all policies to defaults
     */
    static void ResetPoliciesToDefaults();

    /**
     * @brief Get default policy for a specific scenario
     * @param scenario Scenario type ("HIGH_PERFORMANCE", "COMPREHENSIVE", "BALANCED")
     * @param application_name Application name
     * @return Default policy for the scenario
     */
    static PersistencePolicy GetDefaultPolicyForScenario(const std::string& scenario,
                                                        const std::string& application_name);

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Convert decision result to string
     * @param result Decision result to format
     * @return Human-readable decision description
     */
    static std::string DecisionResultToString(const PersistenceDecisionResult& result);

    /**
     * @brief Convert policy to string
     * @param policy Policy to format
     * @return Human-readable policy description
     */
    static std::string PolicyToString(const PersistencePolicy& policy);

    /**
     * @brief Convert context to string
     * @param context Context to format
     * @return Human-readable context description
     */
    static std::string ContextToString(const PersistenceDecisionContext& context);

    /**
     * @brief Get current system metrics for context creation
     * @return Current system performance metrics
     */
    static void GetCurrentSystemMetrics(DWORD& cpu_usage, DWORD& memory_usage,
                                       DWORD& disk_usage, DWORD& error_rate);

private:
    // Private helper methods
    static PersistenceDecisionResult CreateDecisionResult(bool should_persist,
                                                         const std::string& method,
                                                         const std::string& reason);
    static bool IsComponentCritical(const std::string& component, const PersistencePolicy& policy);
    static bool IsComponentIgnored(const std::string& component, const PersistencePolicy& policy);
    static DWORD GetCurrentTimestamp();
    static uint64_t GetCurrentTimeMicroseconds();
    static void InitializeDefaultPolicies();
    static bool IsInitialized();
    static std::string GeneratePolicyId();
};

#endif // __ASFM_LOGGER_CONTEXTUAL_PERSISTENCE_TOOLBOX_HPP__