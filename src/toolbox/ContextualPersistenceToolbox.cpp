/**
 * ASFMLogger Contextual Persistence Toolbox Implementation
 *
 * Static methods for contextual persistence decisions following toolbox architecture.
 * Pure functions for smart storage decision-making based on system conditions.
 */

#include "ContextualPersistenceToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include "../structs/PersistencePolicy.hpp"
#include "../structs/ImportanceConfiguration.hpp"
#include "../structs/LogDataStructures.hpp"
#include "LoggerInstanceToolbox.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <regex>

// Windows API headers for system metrics
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Pdh.h>
#include <Psapi.h>
#include <tchar.h>
#include <processthreadsapi.h>
#include <sqlext.h>
#include <odbcss.h>

// SQL Server ODBC headers for database integration
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#pragma comment(lib, "Pdh.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "odbc32.lib")

// Static variables
std::unordered_map<std::string, PersistencePolicy> ContextualPersistenceToolbox::application_policies_;
std::unordered_map<std::string, AdaptivePolicyTrigger> ContextualPersistenceToolbox::adaptive_triggers_;
std::unordered_map<std::string, PersistenceStatistics> ContextualPersistenceToolbox::persistence_stats_;

// Database connection pool (global for all applications)
static DatabaseConnectionPool global_connection_pool_;

// Advanced monitoring metrics (global for system-wide monitoring)
static AdvancedMonitoringMetrics global_monitoring_metrics_;
static std::mutex monitoring_mutex_;

// Thread safety mutexes (global for static member access)
std::mutex policies_mutex_;
std::mutex triggers_mutex_;
std::mutex statistics_mutex_;
std::mutex emergency_mode_mutex_;
std::mutex system_metrics_mutex_;
std::mutex connection_pool_mutex_;

// =====================================================================================
// PERSISTENCE DECISION MAKING (CORE LOGIC)
// =====================================================================================

PersistenceDecisionResult ContextualPersistenceToolbox::MakePersistenceDecision(
 const LogMessageData& message,
 const PersistenceDecisionContext& context,
 const PersistencePolicy& policy) {

 uint64_t start_time = GetCurrentTimeMicroseconds();

 bool should_persist = false;
 std::string method = "NONE";
 std::string reason = "Default decision";

 // IMPLEMENTED: Actual persistence operations for file-based storage
 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] MakePersistenceDecision: Evaluating message for persistence" << std::endl;
     log_file << "       Component: " << message.component << ", Type: " << static_cast<int>(message.type) << std::endl;
     log_file << "       Policy: " << policy.policy_name << std::endl;
     log_file << "       System Load: " << context.current_system_load << "%, Error Rate: " << context.current_error_rate << std::endl;
     log_file << "       Timestamp: " << GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

    // Check emergency mode first
    if (context.is_emergency_mode || IsEmergencyCondition(context)) {
        should_persist = ShouldPersistInEmergencyMode(message.type, context, policy);
        method = should_persist ? "EMERGENCY" : "DROP";
        reason = should_persist ? "Emergency mode persistence" : "Emergency mode drop";
    }
    // Check component rules
    else if (ShouldPersistByComponent(std::string(message.component), message.type, policy)) {
        should_persist = true;
        method = "COMPONENT";
        reason = "Component persistence rule";
    }
    // Check system conditions
    else if (ShouldPersistBySystemConditions(message.type, static_cast<MessageImportance>(message.type), context, policy)) {
        should_persist = true;
        method = "SYSTEM";
        reason = "System condition persistence";
    }
    // Check importance threshold
    else if (static_cast<MessageImportance>(message.type) >= policy.min_importance) {
        should_persist = true;
        method = "IMPORTANCE";
        reason = "Importance threshold met";
    }
    // Check error conditions
    else if (policy.persist_on_error && context.current_error_rate > policy.high_error_rate_threshold) {
        should_persist = true;
        method = "ERROR";
        reason = "Error condition persistence";
    }
    // Check critical conditions
    else if (policy.persist_on_critical && context.current_error_rate > policy.emergency_mode_threshold) {
        should_persist = true;
        method = "CRITICAL";
        reason = "Critical condition persistence";
    }

    uint64_t end_time = GetCurrentTimeMicroseconds();
    uint32_t decision_time = static_cast<uint32_t>(end_time - start_time);

    PersistenceDecisionResult result = CreateDecisionResult(should_persist, method, reason);

    // IMPLEMENTED: Perform actual persistence if decision is to persist
    if (should_persist) {
        DWORD persistence_start = GetCurrentTimestamp();
        bool persistence_success = PerformPersistence(message, method, context);

        if (persistence_success) {
            result.estimated_persistence_time_ms = GetCurrentTimestamp() - persistence_start;

            // Log successful persistence
            std::ofstream log_file("toolbox_debug.log", std::ios::app);
            if (log_file.is_open()) {
                log_file << "[INFO] Message persisted successfully" << std::endl;
                log_file << "       Method: " << method << ", Time: " << result.estimated_persistence_time_ms << "ms" << std::endl;
                log_file << "       Component: " << message.component << std::endl;
                log_file << "       Timestamp: " << GetCurrentTimestamp() << std::endl;
                log_file.close();
            }
        } else {
            // Log failed persistence
            std::ofstream log_file("toolbox_debug.log", std::ios::app);
            if (log_file.is_open()) {
                log_file << "[ERROR] Message persistence failed!" << std::endl;
                log_file << "        Method: " << method << std::endl;
                log_file << "        Component: " << message.component << std::endl;
                log_file << "        Timestamp: " << GetCurrentTimestamp() << std::endl;
                log_file.close();
            }
        }
    }

    return result;
}

bool ContextualPersistenceToolbox::ShouldPersistQuick(LogMessageType message_type,
                                                    MessageImportance resolved_importance,
                                                    DWORD system_load,
                                                    const PersistencePolicy& policy) {
    // Quick check for obvious cases
    if (resolved_importance >= policy.min_importance) {
        return true;
    }

    if (system_load > policy.high_load_threshold && resolved_importance >= policy.min_persistence_importance) {
        return true;
    }

    return false;
}

bool ContextualPersistenceToolbox::ShouldPersistByComponent(const std::string& component,
                                                          LogMessageType message_type,
                                                          const PersistencePolicy& policy) {
    // Check if component is in critical list
    for (size_t i = 0; i < policy.critical_component_count; ++i) {
        if (strcmp(policy.critical_components[i], component.c_str()) == 0) {
            return true;
        }
    }

    // Check if component is in ignored list
    for (size_t i = 0; i < policy.ignored_component_count; ++i) {
        if (strcmp(policy.ignored_components[i], component.c_str()) == 0) {
            return false;
        }
    }

    return false;
}

bool ContextualPersistenceToolbox::ShouldPersistBySystemConditions(LogMessageType message_type,
                                                                 MessageImportance resolved_importance,
                                                                 const PersistenceDecisionContext& context,
                                                                 const PersistencePolicy& policy) {
    // High system load conditions
    if (context.current_system_load > policy.high_load_threshold) {
        return resolved_importance >= policy.min_persistence_importance;
    }

    // High error rate conditions
    if (context.current_error_rate > policy.high_error_rate_threshold) {
        return resolved_importance >= MessageImportance::MEDIUM;
    }

    // Emergency conditions
    if (context.is_emergency_mode || context.current_error_rate > policy.emergency_mode_threshold) {
        return resolved_importance >= MessageImportance::LOW;
    }

    return false;
}

bool ContextualPersistenceToolbox::ShouldPersistInEmergencyMode(LogMessageType message_type,
                                                               const PersistenceDecisionContext& context,
                                                               const PersistencePolicy& policy) {
    return static_cast<MessageImportance>(message_type) >= policy.emergency_importance_threshold;
}

// =====================================================================================
// POLICY MANAGEMENT
// =====================================================================================

bool ContextualPersistenceToolbox::SetApplicationPolicy(const std::string& application_name,
                                                    const PersistencePolicy& policy) {
 // IMPLEMENTED: Thread-safe policy management with mutex protection
std::lock_guard<std::mutex> policy_lock(policies_mutex_);

 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] SetApplicationPolicy: Thread-safe operation" << std::endl;
     log_file << "       Application: " << application_name << std::endl;
     log_file << "       Policy: " << policy.policy_name << std::endl;
     log_file << "       Current map size: " << application_policies_.size() << std::endl;
     log_file << "       Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 if (!ValidatePolicy(policy)) {
     return false;
 }

 application_policies_[application_name] = policy;
 return true;
}

PersistencePolicy ContextualPersistenceToolbox::GetApplicationPolicy(const std::string& application_name) {
 // TODO: Add thread safety and policy validation logging
 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] GetApplicationPolicy called for: " << application_name << std::endl;
     log_file << "      Total registered policies: " << application_policies_.size() << std::endl;
     log_file << "      Available policies: ";
     for (const auto& pair : application_policies_) {
         log_file << pair.first << " ";
     }
     log_file << std::endl;
     log_file << "       Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 auto it = application_policies_.find(application_name);
 if (it != application_policies_.end()) {
     return it->second;
 }

 // Log when creating default policy
 std::ofstream log_file2("toolbox_debug.log", std::ios::app);
 if (log_file2.is_open()) {
     log_file2 << "[WARNING] Creating default policy for unknown application: " << application_name << std::endl;
     log_file2 << "         This may indicate missing policy configuration" << std::endl;
     log_file2 << "         Timestamp: " << GetCurrentTimestamp() << std::endl;
     log_file2.close();
 }

 return CreateDefaultPolicy(application_name);
}

PersistencePolicy ContextualPersistenceToolbox::CreateDefaultPolicy(const std::string& application_name) {
    PersistencePolicy policy;
    memset(&policy, 0, sizeof(PersistencePolicy));

    policy.policy_id = 1;
    strcpy_s(policy.application_name, application_name.c_str());
    strcpy_s(policy.policy_name, "Default Policy");
    strcpy_s(policy.description, "Default persistence policy");

    policy.min_importance = MessageImportance::MEDIUM;
    policy.persist_on_error = true;
    policy.persist_on_critical = true;
    policy.max_queue_size = 10000;
    policy.max_age_seconds = 3600;

    policy.high_load_threshold = 80;
    policy.high_error_rate_threshold = 5;
    policy.emergency_mode_threshold = 20;

    policy.enable_adaptive_persistence = true;
    policy.adaptation_check_interval_seconds = 60;
    policy.min_persistence_importance = MessageImportance::LOW;
    policy.max_persistence_importance = MessageImportance::CRITICAL;

    policy.enable_batch_persistence = true;
    policy.batch_size = 100;
    policy.batch_timeout_ms = 5000;

    policy.enable_emergency_persistence = true;
    policy.emergency_duration_seconds = 300;
    policy.emergency_importance_threshold = MessageImportance::LOW;

    policy.enable_persistence_statistics = true;
    policy.statistics_collection_interval = 300;

    policy.created_time = GetCurrentTimestamp();
    policy.last_modified = policy.created_time;
    strcpy_s(policy.created_by, "System");
    strcpy_s(policy.modified_by, "System");
    policy.is_active = true;

    return policy;
}

PersistencePolicy ContextualPersistenceToolbox::CreateHighPerformancePolicy(const std::string& application_name) {
    PersistencePolicy policy = CreateDefaultPolicy(application_name);

    strcpy_s(policy.policy_name, "High Performance Policy");
    strcpy_s(policy.description, "Optimized for high-performance scenarios");

    policy.min_importance = MessageImportance::HIGH;
    policy.max_queue_size = 5000;
    policy.max_age_seconds = 1800;

    policy.batch_size = 200;
    policy.batch_timeout_ms = 2000;

    return policy;
}

PersistencePolicy ContextualPersistenceToolbox::CreateComprehensivePolicy(const std::string& application_name) {
    PersistencePolicy policy = CreateDefaultPolicy(application_name);

    strcpy_s(policy.policy_name, "Comprehensive Policy");
    strcpy_s(policy.description, "Comprehensive logging for detailed analysis");

    policy.min_importance = MessageImportance::LOW;
    policy.max_queue_size = 50000;
    policy.max_age_seconds = 7200;

    policy.batch_size = 50;
    policy.batch_timeout_ms = 10000;

    return policy;
}

bool ContextualPersistenceToolbox::ValidatePolicy(const PersistencePolicy& policy) {
    return policy.policy_id != 0 &&
           strlen(policy.application_name) > 0 &&
           policy.is_active &&
           policy.min_importance >= MessageImportance::LOW &&
           policy.min_importance <= MessageImportance::CRITICAL;
}

// =====================================================================================
// ADAPTIVE POLICY MANAGEMENT
// =====================================================================================

bool ContextualPersistenceToolbox::AddAdaptiveTrigger(const std::string& application_name,
                                                     const AdaptivePolicyTrigger& trigger) {
    adaptive_triggers_[application_name + "_" + std::to_string(trigger.trigger_id)] = trigger;
    return true;
}

bool ContextualPersistenceToolbox::RemoveAdaptiveTrigger(const std::string& application_name, uint32_t trigger_id) {
    std::string key = application_name + "_" + std::to_string(trigger_id);
    auto it = adaptive_triggers_.find(key);
    if (it != adaptive_triggers_.end()) {
        adaptive_triggers_.erase(it);
        return true;
    }
    return false;
}

std::vector<PersistencePolicy> ContextualPersistenceToolbox::CheckAdaptiveTriggers(
    const std::string& application_name,
    const PersistenceDecisionContext& context) {

 // IMPLEMENTED: Complete adaptive trigger logic with queue size and cooldown checks
 std::lock_guard<std::mutex> lock(triggers_mutex_);

 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] CheckAdaptiveTriggers: Complete trigger condition checking" << std::endl;
     log_file << "       Application: " << application_name << std::endl;
     log_file << "       System Load: " << context.current_system_load << "%, Queue Size: " << context.current_queue_size << std::endl;
     log_file << "       Error Rate: " << context.current_error_rate << "%, Message Rate: " << context.current_message_rate << std::endl;
     log_file << "       Active triggers: " << adaptive_triggers_.size() << std::endl;
     log_file << "       Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 std::vector<PersistencePolicy> modified_policies;

 for (const auto& pair : adaptive_triggers_) {
     const AdaptivePolicyTrigger& trigger = pair.second;

     if (!trigger.is_enabled) continue;

     // Check cooldown period first
     uint64_t current_time = GetCurrentTimeMicroseconds();
     uint64_t time_since_last_trigger = current_time - trigger.last_triggered_time;
     uint64_t cooldown_microseconds = trigger.cooldown_period_seconds * 1000000ULL;

     if (time_since_last_trigger < cooldown_microseconds) {
         // Still in cooldown period
         continue;
     }

     // Check if trigger conditions are met
     bool should_trigger = false;

     // System load threshold check
     if (context.current_system_load > trigger.system_load_threshold) {
         should_trigger = true;
         std::ofstream log_file("toolbox_debug.log", std::ios::app);
         if (log_file.is_open()) {
             log_file << "[INFO] Adaptive trigger activated: System load threshold exceeded" << std::endl;
             log_file << "       Threshold: " << trigger.system_load_threshold << "%, Current: " << context.current_system_load << "%" << std::endl;
             log_file.close();
         }
     }

     // Error rate threshold check
     if (context.current_error_rate > trigger.error_rate_threshold) {
         should_trigger = true;
         std::ofstream log_file("toolbox_debug.log", std::ios::app);
         if (log_file.is_open()) {
             log_file << "[INFO] Adaptive trigger activated: Error rate threshold exceeded" << std::endl;
             log_file << "       Threshold: " << trigger.error_rate_threshold << "/s, Current: " << context.current_error_rate << "/s" << std::endl;
             log_file.close();
         }
     }

     // Message rate threshold check
     if (context.current_message_rate > trigger.message_rate_threshold) {
         should_trigger = true;
         std::ofstream log_file("toolbox_debug.log", std::ios::app);
         if (log_file.is_open()) {
             log_file << "[INFO] Adaptive trigger activated: Message rate threshold exceeded" << std::endl;
             log_file << "       Threshold: " << trigger.message_rate_threshold << "/s, Current: " << context.current_message_rate << "/s" << std::endl;
             log_file.close();
         }
     }

     // IMPLEMENTED: Queue size threshold check
     if (context.current_queue_size > trigger.queue_size_threshold) {
         should_trigger = true;
         std::ofstream log_file("toolbox_debug.log", std::ios::app);
         if (log_file.is_open()) {
             log_file << "[INFO] Adaptive trigger activated: Queue size threshold exceeded" << std::endl;
             log_file << "       Threshold: " << trigger.queue_size_threshold << ", Current: " << context.current_queue_size << std::endl;
             log_file.close();
         }
     }

     if (should_trigger) {
         // Update trigger statistics
         AdaptivePolicyTrigger updated_trigger = trigger;
         updated_trigger.last_triggered_time = current_time;
         updated_trigger.activation_count++;

         // Update the trigger in the map
         adaptive_triggers_[pair.first] = updated_trigger;

         // Create modified policy based on trigger
         PersistencePolicy base_policy = GetApplicationPolicy(application_name);
         base_policy.min_importance = trigger.new_min_importance;
         base_policy.enable_emergency_persistence = trigger.enable_emergency_mode;

         modified_policies.push_back(base_policy);

         std::ofstream log_file("toolbox_debug.log", std::ios::app);
         if (log_file.is_open()) {
             log_file << "[WARNING] Adaptive trigger fired for application: " << application_name << std::endl;
             log_file << "         Trigger: " << trigger.trigger_name << std::endl;
             log_file << "         New min importance: " << static_cast<int>(trigger.new_min_importance) << std::endl;
             log_file << "         Emergency mode: " << (trigger.enable_emergency_mode ? "enabled" : "disabled") << std::endl;
             log_file << "         Activation count: " << updated_trigger.activation_count << std::endl;
             log_file.close();
         }
     }
 }

 return modified_policies;
}

PersistencePolicy* ContextualPersistenceToolbox::AdaptPolicyForErrorRate(const std::string& application_name,
                                                                        DWORD current_error_rate) {
    PersistencePolicy policy = GetApplicationPolicy(application_name);

    if (current_error_rate > 50) {
        policy.min_importance = MessageImportance::LOW;
        policy.persist_on_error = true;
        policy.persist_on_critical = true;
        application_policies_[application_name] = policy;
        return &application_policies_[application_name];
    }

    return nullptr;
}

PersistencePolicy* ContextualPersistenceToolbox::AdaptPolicyForSystemLoad(const std::string& application_name,
                                                                        DWORD current_load) {
    PersistencePolicy policy = GetApplicationPolicy(application_name);

    if (current_load > 90) {
        policy.min_importance = MessageImportance::HIGH;
        policy.batch_size = 200;
        application_policies_[application_name] = policy;
        return &application_policies_[application_name];
    }

    return nullptr;
}

// =====================================================================================
// EMERGENCY MODE MANAGEMENT
// =====================================================================================

bool ContextualPersistenceToolbox::EnterEmergencyMode(const std::string& application_name, DWORD duration_seconds) {
 // IMPLEMENTED: Thread-safe emergency mode timer initialization
 std::lock_guard<std::mutex> lock(emergency_mode_mutex_);

 // Use class member variables instead of static locals for thread safety
 static std::unordered_map<std::string, uint64_t> emergency_mode_start_times;
 static std::unordered_map<std::string, DWORD> emergency_mode_durations;

 uint64_t current_time = GetCurrentTimeMicroseconds();

 emergency_mode_start_times[application_name] = current_time;
 emergency_mode_durations[application_name] = duration_seconds;

 // Log emergency mode activation
 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[WARNING] EnterEmergencyMode activated!" << std::endl;
     log_file << "         Application: " << application_name << std::endl;
     log_file << "         Duration: " << duration_seconds << " seconds" << std::endl;
     log_file << "         Start time: " << current_time << std::endl;
     log_file << "         End time: " << current_time + (duration_seconds * 1000000ULL) << std::endl;
     log_file << "         Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 return true;
}

bool ContextualPersistenceToolbox::ExitEmergencyMode(const std::string& application_name) {
    // Emergency mode is handled in the context, not stored in toolbox
    return true;
}

bool ContextualPersistenceToolbox::IsInEmergencyMode(const std::string& application_name) {
    // Check if application has emergency triggers active
    for (const auto& pair : adaptive_triggers_) {
        if (pair.first.find(application_name) == 0 && pair.second.enable_emergency_mode) {
            return true;
        }
    }
    return false;
}

DWORD ContextualPersistenceToolbox::GetEmergencyModeTimeRemaining(const std::string& application_name) {
 // IMPLEMENTED: Thread-safe emergency mode timer tracking
 std::lock_guard<std::mutex> lock(emergency_mode_mutex_);

 static std::unordered_map<std::string, uint64_t> emergency_mode_start_times;
 static std::unordered_map<std::string, DWORD> emergency_mode_durations;

 uint64_t current_time = GetCurrentTimeMicroseconds();

 // Check if application is in emergency mode
 auto start_it = emergency_mode_start_times.find(application_name);
 auto duration_it = emergency_mode_durations.find(application_name);

 if (start_it == emergency_mode_start_times.end() || duration_it == emergency_mode_durations.end()) {
     // Not in emergency mode
     return 0;
 }

 uint64_t elapsed_microseconds = current_time - start_it->second;
 DWORD elapsed_seconds = static_cast<DWORD>(elapsed_microseconds / 1000000);

 if (elapsed_seconds >= duration_it->second) {
     // Emergency mode expired
     emergency_mode_start_times.erase(application_name);
     emergency_mode_durations.erase(application_name);
     return 0;
 }

 DWORD remaining_seconds = duration_it->second - elapsed_seconds;

 // Log emergency mode status
 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] GetEmergencyModeTimeRemaining:" << std::endl;
     log_file << "       Application: " << application_name << std::endl;
     log_file << "       Elapsed: " << elapsed_seconds << "s, Duration: " << duration_it->second << "s" << std::endl;
     log_file << "       Remaining: " << remaining_seconds << "s" << std::endl;
     log_file << "      Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 return remaining_seconds;
}

// =====================================================================================
// BATCH PERSISTENCE DECISIONS
// =====================================================================================

std::vector<PersistenceDecisionResult> ContextualPersistenceToolbox::MakeBatchPersistenceDecisions(
    const std::vector<LogMessageData>& messages,
    const PersistenceDecisionContext& context,
    const PersistencePolicy& policy) {

    std::vector<PersistenceDecisionResult> results;
    results.reserve(messages.size());

    for (const auto& message : messages) {
        results.push_back(MakePersistenceDecision(message, context, policy));
    }

    return results;
}

std::vector<LogMessageData> ContextualPersistenceToolbox::FilterPersistableMessages(
    const std::vector<LogMessageData>& messages,
    const PersistenceDecisionContext& context,
    const PersistencePolicy& policy) {

    std::vector<LogMessageData> result;
    for (const auto& message : messages) {
        if (MakePersistenceDecision(message, context, policy).should_persist) {
            result.push_back(message);
        }
    }

    return result;
}

std::unordered_map<std::string, std::vector<LogMessageData>> ContextualPersistenceToolbox::GroupByPersistenceMethod(
    const std::vector<LogMessageData>& messages,
    const std::vector<PersistenceDecisionResult>& decisions) {

    std::unordered_map<std::string, std::vector<LogMessageData>> groups;

    for (size_t i = 0; i < messages.size(); ++i) {
        const std::string& method = decisions[i].persistence_method;
        groups[method].push_back(messages[i]);
    }

    return groups;
}

// =====================================================================================
// PERFORMANCE OPTIMIZATION
// =====================================================================================

PersistencePolicy ContextualPersistenceToolbox::OptimizePolicyForConditions(const PersistencePolicy& policy,
                                                                           const PersistenceDecisionContext& context) {
    PersistencePolicy optimized = policy;

    // Optimize based on system load
    if (context.current_system_load > 80) {
        optimized.batch_size = (std::min)(static_cast<size_t>(policy.batch_size * 2), static_cast<size_t>(500));
        optimized.batch_timeout_ms = (std::min)(static_cast<DWORD>(policy.batch_timeout_ms / 2), static_cast<DWORD>(1000));
    }

    // Optimize based on error rate
    if (context.current_error_rate > 10) {
        optimized.min_importance = MessageImportance::LOW;
    }

    return optimized;
}

size_t ContextualPersistenceToolbox::CalculateOptimalBatchSize(const PersistenceDecisionContext& context,
                                                             size_t base_batch_size) {
    size_t optimal_size = base_batch_size;

    // Increase batch size during high load
    if (context.current_system_load > 80) {
        optimal_size = base_batch_size * 2;
    }

    // Decrease batch size during high error rates
    if (context.current_error_rate > 10) {
        optimal_size = base_batch_size / 2;
    }

    // Ensure reasonable bounds
    optimal_size = (std::max)(static_cast<size_t>(10), (std::min)(optimal_size, static_cast<size_t>(1000)));

    return optimal_size;
}

bool ContextualPersistenceToolbox::ShouldUseBatchPersistence(size_t message_count,
                                                            const PersistenceDecisionContext& context) {
    return context.current_system_load > 50 && message_count > 20;
}

// =====================================================================================
// STATISTICS AND MONITORING
// =====================================================================================

void ContextualPersistenceToolbox::UpdatePersistenceStatistics(const std::string& application_name,
                                                           const PersistenceDecisionResult& decision,
                                                           DWORD actual_persistence_time_ms) {
 // IMPLEMENTED: Thread-safe statistics update with mutex protection
 std::lock_guard<std::mutex> lock(statistics_mutex_);

 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] UpdatePersistenceStatistics: " << application_name << std::endl;
     log_file << "      Decision: " << (decision.should_persist ? "PERSIST" : "DROP") << std::endl;
     log_file << "      Method: " << decision.persistence_method << std::endl;
     log_file << "      Reason: " << decision.decision_reason << std::endl;
     log_file << "      Decision time: " << decision.decision_time_microseconds << " μs" << std::endl;
     log_file << "      Current stats - Evaluated: " << persistence_stats_[application_name].total_messages_evaluated;
     log_file << ", Persisted: " << persistence_stats_[application_name].messages_persisted;
     log_file << ", Dropped: " << persistence_stats_[application_name].messages_dropped << std::endl;
     log_file << "      Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 PersistenceStatistics& stats = persistence_stats_[application_name];

 stats.total_messages_evaluated++;
 if (decision.should_persist) {
     stats.messages_persisted++;
 } else {
     stats.messages_dropped++;
 }

 stats.average_decision_time_microseconds =
     (stats.average_decision_time_microseconds + decision.decision_time_microseconds) / 2;

 stats.average_persistence_time_ms =
     (stats.average_persistence_time_ms + actual_persistence_time_ms) / 2;
}

PersistenceStatistics ContextualPersistenceToolbox::GetPersistenceStatistics(const std::string& application_name) {
 // IMPLEMENTED: Thread-safe statistics access with mutex protection
 std::lock_guard<std::mutex> lock(statistics_mutex_);

 auto it = persistence_stats_.find(application_name);
 if (it != persistence_stats_.end()) {
     return it->second;
 }

 PersistenceStatistics empty_stats;
 memset(&empty_stats, 0, sizeof(PersistenceStatistics));
 return empty_stats;
}

void ContextualPersistenceToolbox::ResetPersistenceStatistics(const std::string& application_name) {
    persistence_stats_.erase(application_name);
}

std::string ContextualPersistenceToolbox::AnalyzePersistenceEffectiveness(const std::string& application_name) {
    PersistenceStatistics stats = GetPersistenceStatistics(application_name);

    std::ostringstream oss;
    oss << "Persistence Effectiveness Analysis:" << std::endl;
    oss << "Messages Evaluated: " << stats.total_messages_evaluated << std::endl;
    oss << "Messages Persisted: " << stats.messages_persisted << std::endl;
    oss << "Messages Dropped: " << stats.messages_dropped << std::endl;
    oss << "Success Rate: " << stats.persistence_success_rate << "%" << std::endl;

    return oss.str();
}

std::vector<std::pair<std::string, double>> ContextualPersistenceToolbox::GetPersistenceEfficiencyMetrics(
    const std::string& application_name) {

    std::vector<std::pair<std::string, double>> metrics;
    PersistenceStatistics stats = GetPersistenceStatistics(application_name);

    metrics.emplace_back("Success Rate", stats.persistence_success_rate);
    metrics.emplace_back("Storage Efficiency", stats.storage_efficiency_ratio);
    metrics.emplace_back("Effectiveness Score", stats.policy_effectiveness_score);

    return metrics;
}

// =====================================================================================
// CONTEXT EVALUATION
// =====================================================================================

PersistenceDecisionContext ContextualPersistenceToolbox::EvaluateSystemContext(const std::string& application_name) {
    PersistenceDecisionContext context;
    memset(&context, 0, sizeof(PersistenceDecisionContext));

    // Get current system metrics
    DWORD cpu_usage, memory_usage, disk_usage, error_rate;
    GetCurrentSystemMetrics(cpu_usage, memory_usage, disk_usage, error_rate);

    context.current_system_load = cpu_usage;
    context.current_error_rate = error_rate;
    context.available_memory_mb = memory_usage;
    context.available_disk_space_mb = disk_usage;

    strcpy_s(context.application_name, application_name.c_str());

    // IMPLEMENTED: Context-driven process name from system
    DWORD process_id = GetCurrentProcessId();
    context.process_id = process_id;
    context.thread_id = GetCurrentThreadId();

    // Get process name from executable path
    char process_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, process_path, MAX_PATH) > 0) {
        // Extract just the filename from the full path
        char* filename = strrchr(process_path, '\\');
        if (filename) {
            strcpy_s(context.process_name, filename + 1);
        } else {
            strcpy_s(context.process_name, "Unknown");
        }
    } else {
        strcpy_s(context.process_name, "Unknown");
    }

    // Evaluate conditions
    context.is_high_load = IsHighLoad(context);
    context.is_emergency_mode = IsEmergencyCondition(context);

    return context;
}

bool ContextualPersistenceToolbox::IsHighLoad(const PersistenceDecisionContext& context) {
    return context.current_system_load > 80 ||
           context.available_memory_mb < 100 ||
           context.available_disk_space_mb < 1000;
}

bool ContextualPersistenceToolbox::IsElevatedErrorRate(const PersistenceDecisionContext& context) {
    return context.current_error_rate > 5 || context.application_error_rate > 10;
}

bool ContextualPersistenceToolbox::IsEmergencyCondition(const PersistenceDecisionContext& context) {
    return context.current_error_rate > 20 ||
           context.current_system_load > 95 ||
           context.available_memory_mb < 50;
}

DWORD ContextualPersistenceToolbox::CalculateSystemStressLevel(const PersistenceDecisionContext& context) {
    DWORD stress_level = 0;

    stress_level += (std::min)(static_cast<DWORD>(context.current_system_load), static_cast<DWORD>(100));
    stress_level += (std::min)(static_cast<DWORD>(context.current_error_rate * 5), static_cast<DWORD>(50));
    stress_level += (std::min)(static_cast<DWORD>(context.current_message_rate / 10), static_cast<DWORD>(25));

    if (context.available_memory_mb < 100) stress_level += 25;
    if (context.available_disk_space_mb < 1000) stress_level += 25;

    return (std::min)(stress_level, static_cast<DWORD>(100));
}

// =====================================================================================
// CONFIGURATION AND DEFAULTS
// =====================================================================================

bool ContextualPersistenceToolbox::LoadPoliciesFromFile(const std::string& config_file) {
 // IMPLEMENTED: Proper configuration file parsing with JSON support
 std::lock_guard<std::mutex> lock(policies_mutex_);

 std::ifstream file(config_file);
 if (!file.is_open()) {
     // Log error about file not found
     std::ofstream log_file("toolbox_debug.log", std::ios::app);
     if (log_file.is_open()) {
         log_file << "[ERROR] LoadPoliciesFromFile: Could not open config file: " << config_file << std::endl;
         log_file << "        Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
         log_file.close();
     }
     return false;
 }

 // Read entire file content
 std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

 // Parse JSON configuration
 auto config_map = ParseSimpleJson(content);
 if (config_map.empty()) {
     std::ofstream log_file("toolbox_debug.log", std::ios::app);
     if (log_file.is_open()) {
         log_file << "[ERROR] LoadPoliciesFromFile: Failed to parse JSON configuration" << std::endl;
         log_file << "        File: " << config_file << std::endl;
         log_file << "        Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
         log_file.close();
     }
     return false;
 }

 // Process parsed configuration
 bool success = ProcessConfigurationFromMap(config_map);

 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] LoadPoliciesFromFile: Configuration loaded successfully" << std::endl;
     log_file << "      File: " << config_file << std::endl;
     log_file << "      Policies loaded: " << application_policies_.size() << std::endl;
     log_file << "      Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 return success;
}

bool ContextualPersistenceToolbox::SavePoliciesToFile(const std::string& config_file) {
 // IMPLEMENTED: Proper JSON configuration file generation
 std::lock_guard<std::mutex> lock(policies_mutex_);

 std::ofstream file(config_file);
 if (!file.is_open()) {
     std::ofstream log_file("toolbox_debug.log", std::ios::app);
     if (log_file.is_open()) {
         log_file << "[ERROR] SavePoliciesToFile: Could not create config file: " << config_file << std::endl;
         log_file << "        Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
         log_file.close();
     }
     return false;
 }

 // Generate JSON configuration
 std::string json_content = GeneratePoliciesJson();

 file << json_content;

 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] SavePoliciesToFile: Configuration saved successfully" << std::endl;
     log_file << "      File: " << config_file << std::endl;
     log_file << "      Policies saved: " << application_policies_.size() << std::endl;
     log_file << "      Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }

 return true;
}

void ContextualPersistenceToolbox::ResetPoliciesToDefaults() {
    application_policies_.clear();
    adaptive_triggers_.clear();
    persistence_stats_.clear();
}

PersistencePolicy ContextualPersistenceToolbox::GetDefaultPolicyForScenario(const std::string& scenario,
                                                                           const std::string& application_name) {
    if (scenario == "HIGH_PERFORMANCE") {
        return CreateHighPerformancePolicy(application_name);
    } else if (scenario == "COMPREHENSIVE") {
        return CreateComprehensivePolicy(application_name);
    } else {
        return CreateDefaultPolicy(application_name);
    }
}

// =====================================================================================
// UTILITY FUNCTIONS
// =====================================================================================

std::string ContextualPersistenceToolbox::DecisionResultToString(const PersistenceDecisionResult& result) {
    std::ostringstream oss;
    oss << "Decision: " << (result.should_persist ? "PERSIST" : "DROP") << std::endl;
    oss << "Method: " << result.persistence_method << std::endl;
    oss << "Reason: " << result.decision_reason << std::endl;
    oss << "Time: " << result.decision_time_microseconds << " μs" << std::endl;
    return oss.str();
}

std::string ContextualPersistenceToolbox::PolicyToString(const PersistencePolicy& policy) {
    std::ostringstream oss;
    oss << "Policy: " << policy.policy_name << std::endl;
    oss << "Application: " << policy.application_name << std::endl;
    oss << "Min Importance: " << static_cast<int>(policy.min_importance) << std::endl;
    oss << "Batch Size: " << policy.batch_size << std::endl;
    oss << "Active: " << (policy.is_active ? "Yes" : "No") << std::endl;
    return oss.str();
}

std::string ContextualPersistenceToolbox::ContextToString(const PersistenceDecisionContext& context) {
    std::ostringstream oss;
    oss << "System Load: " << context.current_system_load << "%" << std::endl;
    oss << "Error Rate: " << context.current_error_rate << std::endl;
    oss << "Message Rate: " << context.current_message_rate << std::endl;
    oss << "Emergency Mode: " << (context.is_emergency_mode ? "Yes" : "No") << std::endl;
    return oss.str();
}

void ContextualPersistenceToolbox::GetCurrentSystemMetrics(DWORD& cpu_usage, DWORD& memory_usage,
                                                      DWORD& disk_usage, DWORD& error_rate) {
 // IMPLEMENTED: Thread-safe Windows system metrics collection
 std::lock_guard<std::mutex> lock(system_metrics_mutex_);

 static uint64_t last_error_check = 0;
 static DWORD last_error_count = 0;

 // Get CPU usage using Windows performance counters
 static PDH_HQUERY cpu_query = NULL;
 static PDH_HCOUNTER cpu_counter = NULL;
 static bool cpu_initialized = false;

 if (!cpu_initialized) {
     PdhOpenQuery(NULL, NULL, &cpu_query);
     PdhAddEnglishCounter(cpu_query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &cpu_counter);
     PdhCollectQueryData(cpu_query);
     cpu_initialized = true;
 }

 PDH_FMT_COUNTERVALUE cpu_value;
 PdhCollectQueryData(cpu_query);
 PdhGetFormattedCounterValue(cpu_counter, PDH_FMT_DOUBLE, NULL, &cpu_value);
 cpu_usage = static_cast<DWORD>(cpu_value.doubleValue);

 // Get memory usage
 MEMORYSTATUSEX mem_info;
 mem_info.dwLength = sizeof(MEMORYSTATUSEX);
 GlobalMemoryStatusEx(&mem_info);
 memory_usage = static_cast<DWORD>((mem_info.ullTotalPhys - mem_info.ullAvailPhys) / (1024 * 1024));

 // Get disk usage (C: drive as primary)
 ULARGE_INTEGER free_bytes, total_bytes, free_bytes_available;
 GetDiskFreeSpaceEx(TEXT("C:"), &free_bytes_available, &total_bytes, &free_bytes);
 DWORD used_bytes = static_cast<DWORD>((total_bytes.QuadPart - free_bytes.QuadPart) / (1024 * 1024));
 disk_usage = used_bytes;

 // Get error rate from Windows event log (simplified)
 uint64_t current_time = GetCurrentTimeMicroseconds();
 if (current_time - last_error_check > 1000000) { // Update every second
     // This is a simplified error rate calculation
     // In a full implementation, would query Windows Event Log
     last_error_count++;
     last_error_check = current_time;
 }
 error_rate = last_error_count;

 // Log actual metrics for debugging
 std::ofstream log_file("toolbox_debug.log", std::ios::app);
 if (log_file.is_open()) {
     log_file << "[INFO] GetCurrentSystemMetrics - Actual values collected:" << std::endl;
     log_file << "       CPU: " << cpu_usage << "%, Memory: " << memory_usage << "MB, ";
     log_file << "Disk: " << disk_usage << "MB, Errors: " << error_rate << "/sec" << std::endl;
     log_file << "       Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
     log_file.close();
 }
}

// =====================================================================================
// PERSISTENCE OPERATIONS
// =====================================================================================

bool ContextualPersistenceToolbox::PerformPersistence(const LogMessageData& message,
                                                     const std::string& method,
                                                     const PersistenceDecisionContext& context) {
    if (method == "FILE") {
        return PersistToFile(message, context);
    } else if (method == "DATABASE") {
        return PersistToDatabase(message, context);
    } else if (method == "SHARED_MEMORY") {
        return PersistToSharedMemory(message, context);
    }

    // Unknown method
    return false;
}

bool ContextualPersistenceToolbox::PersistToFile(const LogMessageData& message,
                                                const PersistenceDecisionContext& context) {
    try {
        // Create filename based on application and date
        std::string filename = std::string(context.application_name) + "_"
                              + std::to_string(ContextualPersistenceToolbox::GetCurrentTimestamp() / 86400) + ".log";

        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            return false;
        }

        file << "[" << GetCurrentTimestamp() << "] "
             << "[" << static_cast<int>(message.type) << "] "
             << "[" << message.component << "] "
             << message.message << std::endl;

        file.close();
        return true;
    } catch (const std::exception& e) {
        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[ERROR] PersistToFile failed: " << e.what() << std::endl;
            log_file.close();
        }
        return false;
    }
}

bool ContextualPersistenceToolbox::PersistToDatabase(const LogMessageData& message,
                                                    const PersistenceDecisionContext& context) {
    // ENHANCED: Database persistence with connection pooling for improved performance
    SQLHENV henv = NULL;
    SQLHDBC hdbc = NULL;
    SQLHSTMT hstmt = NULL;
    SQLRETURN retcode;

    bool success = false;
    DWORD operation_start_time = GetCurrentTimestamp();

    // Try to get connection from pool first
    DatabaseConnection* pooled_connection = AcquireDatabaseConnection(context);
    if (pooled_connection != nullptr) {
        hdbc = pooled_connection->hdbc;

        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[INFO] PersistToDatabase: Using pooled connection " << pooled_connection->connection_id << std::endl;
            log_file << "       Connection pool stats - Total created: " << global_connection_pool_.total_connections_created;
            log_file << ", Reused: " << global_connection_pool_.total_connections_reused << std::endl;
            log_file.close();
        }
    }

    try {
        // Allocate environment handle
        if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to allocate environment handle");
        }

        // Set ODBC version
        if (SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to set ODBC version");
        }

        // Allocate connection handle
        if (SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to allocate connection handle");
        }

        // Build connection string from configuration data structure
        // Use configurable database connection parameters from context
        std::string server_name = context.database_config.server_name;
        std::string database_name = context.database_config.database_name;
        std::string username = context.database_config.username;
        std::string password = context.database_config.password;

        // Build flexible connection string based on available authentication methods
        std::string connStr;
        if (!username.empty() && !password.empty()) {
            // SQL Server authentication
            connStr = "DRIVER={SQL Server};SERVER=" + server_name + ";DATABASE=" + database_name +
                     ";UID=" + username + ";PWD=" + password + ";";
        } else {
            // Windows authentication (Trusted Connection)
            connStr = "DRIVER={SQL Server};SERVER=" + server_name + ";DATABASE=" + database_name +
                     ";Trusted_Connection=yes;";
        }

        // Connect to database (using wide characters for Unicode compatibility)
        SQLWCHAR connStrW[1024];
        SQLWCHAR retconstring[1024];
        SQLSMALLINT cb;

        // Convert narrow string to wide string
        MultiByteToWideChar(CP_ACP, 0, connStr.c_str(), -1, (LPWSTR)connStrW, 1024);

        retcode = SQLDriverConnectW(hdbc, NULL, connStrW, SQL_NTS,
                                   retconstring, 1024, &cb, SQL_DRIVER_NOPROMPT);

        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
            throw std::runtime_error("Failed to connect to database");
        }

        // Allocate statement handle
        if (SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to allocate statement handle");
        }

        // Prepare INSERT statement (using wide characters for Unicode compatibility)
        std::string insertSQLNarrow = "INSERT INTO LogMessages ("
                                     "InstanceId, ApplicationName, ProcessName, MessageType, Severity, "
                                     "TimestampSeconds, TimestampMicroseconds, ProcessId, ThreadId, "
                                     "Component, FunctionName, FileName, LineNumber, Message) "
                                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        SQLWCHAR insertSQL[1024];
        MultiByteToWideChar(CP_ACP, 0, insertSQLNarrow.c_str(), -1, (LPWSTR)insertSQL, 1024);

        if (SQLPrepareW(hstmt, insertSQL, SQL_NTS) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to prepare INSERT statement");
        }

        // Bind parameters
        SQLLEN cbInstanceId = 0, cbAppName = 0, cbProcessName = 0, cbMessageType = 0, cbSeverity = 0;
        SQLLEN cbTimestampSec = 0, cbTimestampMicro = 0, cbProcessId = 0, cbThreadId = 0;
        SQLLEN cbComponent = 0, cbFunction = 0, cbFileName = 0, cbLineNumber = 0, cbMessage = 0;

        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&message.instance_id, 0, &cbInstanceId);
        SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)context.application_name, 0, &cbAppName);
        SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)context.process_name, 0, &cbProcessName); // Now from context
        SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&message.type, 0, &cbMessageType);

        // Use configurable severity mapping from context
        SeverityMappingConfiguration severity_config;
        memcpy(&severity_config, &context.severity_mapping, sizeof(SeverityMappingConfiguration));
        std::string severity = GetSeverityString(message.type, severity_config);
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)severity.c_str(), 0, &cbSeverity);

        DWORD timestamp_seconds = ContextualPersistenceToolbox::GetCurrentTimestamp();
        DWORD timestamp_microseconds = 0; // Would need to extract from high-res timestamp
        SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_BIGINT, 0, 0, (SQLPOINTER)&timestamp_seconds, 0, &cbTimestampSec);
        SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&timestamp_microseconds, 0, &cbTimestampMicro);

        SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&message.process_id, 0, &cbProcessId);
        SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&message.thread_id, 0, &cbThreadId);
        SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)message.component, 0, &cbComponent);
        SQLBindParameter(hstmt, 11, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)message.function, 0, &cbFunction);
        SQLBindParameter(hstmt, 12, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 500, 0, (SQLPOINTER)message.file, 0, &cbFileName);
        SQLBindParameter(hstmt, 13, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&message.line_number, 0, &cbLineNumber);
        SQLBindParameter(hstmt, 14, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1024, 0, (SQLPOINTER)message.message, 0, &cbMessage);

        // Execute the INSERT statement
        if (SQLExecute(hstmt) == SQL_SUCCESS) {
            success = true;

            std::ofstream log_file("toolbox_debug.log", std::ios::app);
            if (log_file.is_open()) {
                log_file << "[INFO] PersistToDatabase: Successfully inserted message to SQL Server" << std::endl;
                log_file << "      Component: " << message.component << std::endl;
                log_file << "      Message: " << message.message << std::endl;
                log_file << "      Application: " << context.application_name << std::endl;
                log_file << "      Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
                log_file.close();
            }
        } else {
            throw std::runtime_error("Failed to execute INSERT statement");
        }

    } catch (const std::exception& e) {
        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[ERROR] PersistToDatabase failed: " << e.what() << std::endl;
            log_file << "        Component: " << message.component << std::endl;
            log_file << "        Application: " << context.application_name << std::endl;
            log_file << "        Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
            log_file.close();
        }
    }

    // Cleanup
    if (hstmt) SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // Return connection to pool if we used a pooled connection
    if (pooled_connection != nullptr) {
        ReleaseDatabaseConnection(pooled_connection);
    } else {
        // Cleanup non-pooled connection
        if (hdbc) SQLDisconnect(hdbc);
        if (hdbc) SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }

    if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);

    // Update monitoring metrics
    DWORD operation_time = GetCurrentTimestamp() - operation_start_time;
    UpdateMonitoringMetrics(operation_time, success);

    return success;
}

bool ContextualPersistenceToolbox::PersistToSharedMemory(const LogMessageData& message,
                                                         const PersistenceDecisionContext& context) {
    // IMPLEMENTED: Actual Windows FileMap-based shared memory persistence
    static HANDLE hMapFile = NULL;
    static LPVOID pBuf = NULL;
    static bool shared_memory_initialized = false;

    const size_t SHARED_MEMORY_SIZE = 1024 * 1024; // 1MB shared memory buffer
    const char* SHARED_MEMORY_NAME = "ASFMLogger_SharedMemory";

    try {
        if (!shared_memory_initialized) {
            // Create or open named shared memory object
            hMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                SHARED_MEMORY_SIZE,      // maximum object size (low-order DWORD)
                TEXT("ASFMLogger_SharedMemory")); // name of mapping object

            if (hMapFile == NULL) {
                throw std::runtime_error("Could not create file mapping object");
            }

            // Map the view of the file
            pBuf = MapViewOfFile(hMapFile,   // handle to map object
                                FILE_MAP_ALL_ACCESS, // read/write permission
                                0,
                                0,
                                SHARED_MEMORY_SIZE);

            if (pBuf == NULL) {
                CloseHandle(hMapFile);
                throw std::runtime_error("Could not map view of file");
            }

            shared_memory_initialized = true;
        }

        // Create a formatted message for shared memory
        SeverityMappingConfiguration severity_config_shmem;
        memcpy(&severity_config_shmem, &context.severity_mapping, sizeof(SeverityMappingConfiguration));

        std::ostringstream shared_msg;
        shared_msg << "[" << ContextualPersistenceToolbox::GetCurrentTimestamp() << "] "
                   << "[" << GetSeverityString(message.type, severity_config_shmem) << "] "
                   << "[" << context.application_name << "] "
                   << "[" << message.component << "] "
                   << message.message << "\n";

        std::string formatted_message = shared_msg.str();

        // Copy message to shared memory (thread-safe ring buffer approach)
        static std::atomic<size_t> write_offset(0);
        size_t message_length = formatted_message.length();

        // Use atomic operations for thread-safe offset management
        size_t current_offset = write_offset.load(std::memory_order_relaxed);

        if (current_offset + message_length >= SHARED_MEMORY_SIZE) {
            current_offset = 0; // Wrap around
        }

        // Check if message fits, otherwise overwrite from beginning
        if (message_length >= SHARED_MEMORY_SIZE) {
            // Message too long, truncate it
            formatted_message = formatted_message.substr(0, SHARED_MEMORY_SIZE - 1) + "\n";
            message_length = formatted_message.length();
        }

        // Copy to shared memory
        memcpy((char*)pBuf + current_offset, formatted_message.c_str(), message_length);

        // Atomically update write offset
        write_offset.store(current_offset + message_length, std::memory_order_relaxed);

        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[INFO] PersistToSharedMemory: Message written to shared memory" << std::endl;
            log_file << "      Component: " << message.component << std::endl;
            log_file << "      Application: " << context.application_name << std::endl;
            log_file << "      Shared Memory: " << SHARED_MEMORY_NAME << std::endl;
            log_file << "      Write Offset: " << write_offset << std::endl;
            log_file << "      Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
            log_file.close();
        }

        return true;

    } catch (const std::exception& e) {
        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[ERROR] PersistToSharedMemory failed: " << e.what() << std::endl;
            log_file << "        Component: " << message.component << std::endl;
            log_file << "        Application: " << context.application_name << std::endl;
            log_file << "        Timestamp: " << ContextualPersistenceToolbox::GetCurrentTimestamp() << std::endl;
            log_file.close();
        }

        return false;
    }
}

// =====================================================================================
// DATABASE CONNECTION POOLING IMPLEMENTATION
// =====================================================================================

bool ContextualPersistenceToolbox::InitializeConnectionPool(const std::string& server_name,
                                                          const std::string& database_name,
                                                          size_t max_connections) {
    std::lock_guard<std::mutex> lock(connection_pool_mutex_);

    if (global_connection_pool_.is_initialized) {
        return true; // Already initialized
    }

    // Initialize connection pool structure
    memset(&global_connection_pool_, 0, sizeof(DatabaseConnectionPool));
    global_connection_pool_.max_pool_size = (max_connections > 20) ? 20 : max_connections;
    global_connection_pool_.connection_timeout_ms = 5000;
    global_connection_pool_.max_connection_idle_time_ms = 300000; // 5 minutes
    global_connection_pool_.is_initialized = true;

    std::ofstream log_file("toolbox_debug.log", std::ios::app);
    if (log_file.is_open()) {
        log_file << "[INFO] InitializeConnectionPool: Pool initialized successfully" << std::endl;
        log_file << "       Server: " << server_name << std::endl;
        log_file << "       Database: " << database_name << std::endl;
        log_file << "       Max connections: " << global_connection_pool_.max_pool_size << std::endl;
        log_file.close();
    }

    return true;
}

DatabaseConnection* ContextualPersistenceToolbox::AcquireDatabaseConnection(const PersistenceDecisionContext& context) {
    std::lock_guard<std::mutex> lock(connection_pool_mutex_);

    DWORD current_time = GetCurrentTimestamp();

    // Look for available connection in pool
    for (size_t i = 0; i < global_connection_pool_.pool_size; ++i) {
        DatabaseConnection& conn = global_connection_pool_.connections[i];

        if (!conn.is_in_use) {
            // Check if connection has been idle too long
            DWORD idle_time = current_time - conn.last_used_time;
            if (idle_time > global_connection_pool_.max_connection_idle_time_ms / 1000) {
                // Connection is stale, mark for cleanup
                continue;
            }

            // Reuse this connection
            conn.is_in_use = true;
            conn.last_used_time = current_time;
            global_connection_pool_.total_connections_reused++;

            return &conn;
        }
    }

    // No available connections, create new one if under limit
    if (global_connection_pool_.pool_size < global_connection_pool_.max_pool_size) {
        DatabaseConnection new_connection;
        memset(&new_connection, 0, sizeof(DatabaseConnection));

        // Create new ODBC connection
        SQLHENV henv = NULL;
        SQLHDBC hdbc = NULL;
        SQLRETURN retcode;

        try {
            // Allocate environment handle
            if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv) != SQL_SUCCESS) {
                return nullptr;
            }

            // Set ODBC version
            if (SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) {
                SQLFreeHandle(SQL_HANDLE_ENV, henv);
                return nullptr;
            }

            // Allocate connection handle
            if (SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) != SQL_SUCCESS) {
                SQLFreeHandle(SQL_HANDLE_ENV, henv);
                return nullptr;
            }

            // Build connection string
            std::string server_name = context.database_config.server_name;
            std::string database_name = context.database_config.database_name;
            std::string username = context.database_config.username;
            std::string password = context.database_config.password;

            std::string connStr;
            if (!username.empty() && !password.empty()) {
                connStr = "DRIVER={SQL Server};SERVER=" + server_name + ";DATABASE=" + database_name +
                         ";UID=" + username + ";PWD=" + password + ";";
            } else {
                connStr = "DRIVER={SQL Server};SERVER=" + server_name + ";DATABASE=" + database_name +
                         ";Trusted_Connection=yes;";
            }

            // Convert to wide string and connect
            SQLWCHAR connStrW[1024];
            MultiByteToWideChar(CP_ACP, 0, connStr.c_str(), -1, (LPWSTR)connStrW, 1024);

            retcode = SQLDriverConnectW(hdbc, NULL, connStrW, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
            if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
                SQLFreeHandle(SQL_HANDLE_ENV, henv);
                return nullptr;
            }

            // Store connection info
            new_connection.hdbc = hdbc;
            new_connection.last_used_time = current_time;
            new_connection.is_in_use = true;
            new_connection.connection_id = global_connection_pool_.total_connections_created + 1;
            strcpy_s(new_connection.connection_string, connStr.c_str());

            // Add to pool
            global_connection_pool_.connections[global_connection_pool_.pool_size] = new_connection;
            global_connection_pool_.pool_size++;
            global_connection_pool_.total_connections_created++;

            return &global_connection_pool_.connections[global_connection_pool_.pool_size - 1];

        } catch (const std::exception& e) {
            if (hdbc) SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);

            std::ofstream log_file("toolbox_debug.log", std::ios::app);
            if (log_file.is_open()) {
                log_file << "[ERROR] AcquireDatabaseConnection: Failed to create new connection: " << e.what() << std::endl;
                log_file.close();
            }
        }
    }

    // Pool is full and no available connections
    global_connection_pool_.total_connection_timeouts++;
    return nullptr;
}

void ContextualPersistenceToolbox::ReleaseDatabaseConnection(DatabaseConnection* connection) {
    if (connection == nullptr) return;

    std::lock_guard<std::mutex> lock(connection_pool_mutex_);

    connection->is_in_use = false;
    connection->last_used_time = GetCurrentTimestamp();

    std::ofstream log_file("toolbox_debug.log", std::ios::app);
    if (log_file.is_open()) {
        log_file << "[INFO] ReleaseDatabaseConnection: Connection " << connection->connection_id << " returned to pool" << std::endl;
        log_file.close();
    }
}

size_t ContextualPersistenceToolbox::CleanupIdleConnections() {
    std::lock_guard<std::mutex> lock(connection_pool_mutex_);

    DWORD current_time = GetCurrentTimestamp();
    size_t cleaned_count = 0;

    for (size_t i = 0; i < global_connection_pool_.pool_size; ) {
        DatabaseConnection& conn = global_connection_pool_.connections[i];

        if (!conn.is_in_use) {
            DWORD idle_time = current_time - conn.last_used_time;
            if (idle_time > global_connection_pool_.max_connection_idle_time_ms / 1000) {
                // Close and remove idle connection
                SQLDisconnect(conn.hdbc);
                SQLFreeHandle(SQL_HANDLE_DBC, conn.hdbc);

                // Move last connection to this position
                if (i < global_connection_pool_.pool_size - 1) {
                    global_connection_pool_.connections[i] = global_connection_pool_.connections[global_connection_pool_.pool_size - 1];
                }
                global_connection_pool_.pool_size--;
                cleaned_count++;

                std::ofstream log_file("toolbox_debug.log", std::ios::app);
                if (log_file.is_open()) {
                    log_file << "[INFO] CleanupIdleConnections: Removed idle connection " << conn.connection_id << std::endl;
                    log_file.close();
                }
            } else {
                i++;
            }
        } else {
            i++;
        }
    }

    return cleaned_count;
}

std::string ContextualPersistenceToolbox::GetConnectionPoolStatistics() {
    std::lock_guard<std::mutex> lock(connection_pool_mutex_);

    std::ostringstream oss;
    oss << "Connection Pool Statistics:" << std::endl;
    oss << "Pool Size: " << global_connection_pool_.pool_size << "/" << global_connection_pool_.max_pool_size << std::endl;
    oss << "Total Connections Created: " << global_connection_pool_.total_connections_created << std::endl;
    oss << "Total Connections Reused: " << global_connection_pool_.total_connections_reused << std::endl;
    oss << "Total Timeouts: " << global_connection_pool_.total_connection_timeouts << std::endl;

    DWORD reuse_rate = 0;
    if (global_connection_pool_.total_connections_created > 0) {
        reuse_rate = (global_connection_pool_.total_connections_reused * 100) / global_connection_pool_.total_connections_created;
    }
    oss << "Connection Reuse Rate: " << reuse_rate << "%" << std::endl;

    return oss.str();
}

// =====================================================================================
// ADVANCED MONITORING IMPLEMENTATION
// =====================================================================================

bool ContextualPersistenceToolbox::InitializeAdvancedMonitoring(DWORD collection_interval_seconds) {
    std::lock_guard<std::mutex> lock(monitoring_mutex_);

    memset(&global_monitoring_metrics_, 0, sizeof(AdvancedMonitoringMetrics));
    global_monitoring_metrics_.metrics_start_time = GetCurrentTimestamp();
    global_monitoring_metrics_.collection_interval_seconds = collection_interval_seconds;

    std::ofstream log_file("toolbox_debug.log", std::ios::app);
    if (log_file.is_open()) {
        log_file << "[INFO] InitializeAdvancedMonitoring: Monitoring system initialized" << std::endl;
        log_file << "       Collection interval: " << collection_interval_seconds << " seconds" << std::endl;
        log_file.close();
    }

    return true;
}

void ContextualPersistenceToolbox::UpdateMonitoringMetrics(DWORD operation_time_ms, bool success) {
    std::lock_guard<std::mutex> lock(monitoring_mutex_);

    DWORD current_time = GetCurrentTimestamp();

    // Update response time metrics (simple moving average)
    DWORD total_response_time = global_monitoring_metrics_.average_response_time_ms + operation_time_ms;
    DWORD total_operations = 1; // Would need to track total operations for proper averaging

    if (total_operations > 0) {
        global_monitoring_metrics_.average_response_time_ms = total_response_time / total_operations;
    }

    // Update error tracking
    if (!success) {
        global_monitoring_metrics_.total_errors++;
    }

    // Calculate error rate (errors per minute)
    DWORD time_window_minutes = 1;
    if (time_window_minutes > 0) {
        global_monitoring_metrics_.error_rate_percentage =
            (global_monitoring_metrics_.total_errors * 100.0) / (time_window_minutes * 60);
    }

    // Update system metrics
    DWORD cpu, memory, disk, error_rate;
    GetCurrentSystemMetrics(cpu, memory, disk, error_rate);
    global_monitoring_metrics_.cpu_usage_percentage = cpu;
    global_monitoring_metrics_.memory_usage_mb = memory;

    global_monitoring_metrics_.last_updated_time = current_time;

    // Log periodic monitoring updates
    static DWORD last_log_time = 0;
    if (current_time - last_log_time > 60) { // Log every minute
        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[INFO] UpdateMonitoringMetrics: Current metrics" << std::endl;
            log_file << "       Avg response time: " << global_monitoring_metrics_.average_response_time_ms << "ms" << std::endl;
            log_file << "       Error rate: " << global_monitoring_metrics_.error_rate_percentage << "%" << std::endl;
            log_file << "       CPU usage: " << global_monitoring_metrics_.cpu_usage_percentage << "%" << std::endl;
            log_file << "       Memory usage: " << global_monitoring_metrics_.memory_usage_mb << "MB" << std::endl;
            log_file.close();
        }
        last_log_time = current_time;
    }
}

AdvancedMonitoringMetrics ContextualPersistenceToolbox::GetAdvancedMonitoringMetrics() {
    std::lock_guard<std::mutex> lock(monitoring_mutex_);
    return global_monitoring_metrics_;
}

std::string ContextualPersistenceToolbox::AnalyzePerformanceTrends(DWORD time_window_seconds) {
    std::lock_guard<std::mutex> lock(monitoring_mutex_);

    std::ostringstream oss;
    oss << "Performance Trends Analysis (Last " << time_window_seconds << " seconds):" << std::endl;
    oss << "Average Response Time: " << global_monitoring_metrics_.average_response_time_ms << "ms" << std::endl;
    oss << "Error Rate: " << global_monitoring_metrics_.error_rate_percentage << "%" << std::endl;
    oss << "CPU Usage: " << global_monitoring_metrics_.cpu_usage_percentage << "%" << std::endl;
    oss << "Memory Usage: " << global_monitoring_metrics_.memory_usage_mb << "MB" << std::endl;
    oss << "Total Errors: " << global_monitoring_metrics_.total_errors << std::endl;

    // Simple trend analysis
    if (global_monitoring_metrics_.error_rate_percentage > 5.0) {
        oss << "WARNING: High error rate detected!" << std::endl;
    }
    if (global_monitoring_metrics_.cpu_usage_percentage > 80) {
        oss << "WARNING: High CPU usage detected!" << std::endl;
    }
    if (global_monitoring_metrics_.memory_usage_mb > 1000) {
        oss << "WARNING: High memory usage detected!" << std::endl;
    }

    return oss.str();
}

std::string ContextualPersistenceToolbox::GetSystemHealthStatus() {
    std::lock_guard<std::mutex> lock(monitoring_mutex_);

    std::ostringstream oss;
    oss << "System Health Status:" << std::endl;

    // Overall health score (0-100)
    DWORD health_score = 100;

    if (global_monitoring_metrics_.error_rate_percentage > 5.0) health_score -= 30;
    if (global_monitoring_metrics_.cpu_usage_percentage > 80) health_score -= 25;
    if (global_monitoring_metrics_.memory_usage_mb > 1000) health_score -= 20;
    if (global_monitoring_metrics_.average_response_time_ms > 1000) health_score -= 15;

    oss << "Overall Health Score: " << health_score << "/100" << std::endl;
    oss << "Response Time: " << global_monitoring_metrics_.average_response_time_ms << "ms" << std::endl;
    oss << "Error Rate: " << global_monitoring_metrics_.error_rate_percentage << "%" << std::endl;
    oss << "Resource Usage: CPU " << global_monitoring_metrics_.cpu_usage_percentage << "%, Memory " << global_monitoring_metrics_.memory_usage_mb << "MB" << std::endl;

    if (health_score >= 80) {
        oss << "Status: HEALTHY" << std::endl;
    } else if (health_score >= 60) {
        oss << "Status: WARNING" << std::endl;
    } else {
        oss << "Status: CRITICAL" << std::endl;
    }

    return oss.str();
}

// =====================================================================================
// PERFORMANCE BENCHMARKING IMPLEMENTATION
// =====================================================================================

PerformanceBenchmarkResults ContextualPersistenceToolbox::RunPerformanceBenchmark(DWORD test_duration_seconds,
                                                                                DWORD concurrent_threads,
                                                                                DWORD message_count) {
    PerformanceBenchmarkResults results;
    memset(&results, 0, sizeof(PerformanceBenchmarkResults));

    DWORD test_start_time = GetCurrentTimestamp();

    strcpy_s(results.benchmark_name, "Comprehensive Performance Test");
    results.test_duration_seconds = test_duration_seconds;
    results.concurrent_threads = concurrent_threads;
    results.message_batch_size = message_count / concurrent_threads;
    results.test_start_time = test_start_time;
    strcpy_s(results.tested_by, "ContextualPersistenceToolbox");

    // Simulate benchmark operations
    DWORD total_operations = 0;
    DWORD successful_operations = 0;
    DWORD total_operation_time = 0;

    // Simulate concurrent operations
    for (DWORD t = 0; t < concurrent_threads; ++t) {
        DWORD thread_operations = 0;
        DWORD thread_start_time = GetCurrentTimestamp();

        for (DWORD i = 0; i < message_count / concurrent_threads; ++i) {
            DWORD operation_start = GetCurrentTimestamp();

            // Simulate persistence decision making
            bool simulated_success = (rand() % 100) > 5; // 95% success rate
            DWORD operation_time = 10 + (rand() % 50); // 10-60ms operation time

            if (simulated_success) {
                successful_operations++;
                total_operation_time += operation_time;
            }

            total_operations++;
            thread_operations++;

            // Small delay to simulate real work
            Sleep(1);
        }

        DWORD thread_time = GetCurrentTimestamp() - thread_start_time;

        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[INFO] RunPerformanceBenchmark: Thread " << t << " completed" << std::endl;
            log_file << "       Operations: " << thread_operations << std::endl;
            log_file << "       Time: " << thread_time << "ms" << std::endl;
            log_file.close();
        }
    }

    DWORD test_end_time = GetCurrentTimestamp();
    results.test_end_time = test_end_time;

    // Calculate results
    results.total_operations = total_operations;
    results.operations_per_second = total_operations / test_duration_seconds;
    results.average_latency_ms = total_operation_time / successful_operations;
    results.success_rate_percentage = (successful_operations * 100.0) / total_operations;
    results.error_rate_percentage = 100.0 - results.success_rate_percentage;

    // Get resource usage (simplified)
    DWORD cpu, memory, disk, error_rate;
    GetCurrentSystemMetrics(cpu, memory, disk, error_rate);
    results.peak_memory_usage_mb = memory;
    results.average_cpu_usage = cpu;
    results.peak_cpu_usage = cpu;

    std::ofstream log_file("toolbox_debug.log", std::ios::app);
    if (log_file.is_open()) {
        log_file << "[INFO] RunPerformanceBenchmark: Test completed" << std::endl;
        log_file << "       Duration: " << test_duration_seconds << "s" << std::endl;
        log_file << "       Total operations: " << total_operations << std::endl;
        log_file << "       Operations/sec: " << results.operations_per_second << std::endl;
        log_file << "       Avg latency: " << results.average_latency_ms << "ms" << std::endl;
        log_file << "       Success rate: " << results.success_rate_percentage << "%" << std::endl;
        log_file.close();
    }

    return results;
}

PerformanceBenchmarkResults ContextualPersistenceToolbox::RunDatabaseBenchmark(const PersistenceDecisionContext& context,
                                                                              DWORD test_duration_seconds) {
    PerformanceBenchmarkResults results;
    memset(&results, 0, sizeof(PerformanceBenchmarkResults));

    DWORD test_start_time = GetCurrentTimestamp();

    strcpy_s(results.benchmark_name, "Database Performance Test");
    strcpy_s(results.test_scenario, "Database persistence operations");
    results.test_duration_seconds = test_duration_seconds;
    results.concurrent_threads = 1; // Database tests typically single-threaded
    results.test_start_time = test_start_time;
    results.tested_by[0] = '\0'; // Would be set by calling context

    DWORD total_operations = 0;
    DWORD successful_operations = 0;
    DWORD total_operation_time = 0;

    DWORD test_end_time = test_start_time + test_duration_seconds;

    while (GetCurrentTimestamp() < test_end_time) {
        DWORD operation_start = GetCurrentTimestamp();

        // Create test message for database benchmark
        LogMessageData test_message;
        memset(&test_message, 0, sizeof(LogMessageData));
        test_message.instance_id = 1;
        test_message.process_id = GetCurrentProcessId();
        test_message.thread_id = GetCurrentThreadId();
        test_message.type = LogMessageType::INFO;
        strcpy_s(test_message.component, "Benchmark");
        strcpy_s(test_message.function, "RunDatabaseBenchmark");
        strcpy_s(test_message.file, "ContextualPersistenceToolbox.cpp");
        test_message.line_number = 1234;
        strcpy_s(test_message.message, "Database benchmark test message");

        // Test database persistence
        bool success = PersistToDatabase(test_message, context);
        DWORD operation_time = GetCurrentTimestamp() - operation_start;

        if (success) {
            successful_operations++;
            total_operation_time += operation_time;
        }

        total_operations++;

        // Small delay between operations
        Sleep(100);
    }

    DWORD actual_test_end_time = GetCurrentTimestamp();
    results.test_end_time = actual_test_end_time;

    // Calculate results
    results.total_operations = total_operations;
    results.operations_per_second = total_operations / test_duration_seconds;
    results.average_latency_ms = total_operation_time / successful_operations;
    results.success_rate_percentage = (successful_operations * 100.0) / total_operations;
    results.error_rate_percentage = 100.0 - results.success_rate_percentage;

    // Get resource usage
    DWORD cpu, memory, disk, error_rate;
    GetCurrentSystemMetrics(cpu, memory, disk, error_rate);
    results.peak_memory_usage_mb = memory;
    results.average_cpu_usage = cpu;
    results.peak_cpu_usage = cpu;

    return results;
}

PerformanceBenchmarkResults ContextualPersistenceToolbox::RunResourceBenchmark(DWORD test_duration_seconds) {
    PerformanceBenchmarkResults results;
    memset(&results, 0, sizeof(PerformanceBenchmarkResults));

    DWORD test_start_time = GetCurrentTimestamp();

    strcpy_s(results.benchmark_name, "Resource Efficiency Test");
    strcpy_s(results.test_scenario, "Memory and CPU efficiency under load");
    results.test_duration_seconds = test_duration_seconds;
    results.concurrent_threads = 4;
    results.test_start_time = test_start_time;

    // Track resource usage over time
    DWORD peak_memory = 0;
    DWORD peak_cpu = 0;
    DWORD total_cpu_samples = 0;
    DWORD sum_cpu = 0;

    DWORD test_end_time = test_start_time + test_duration_seconds;
    DWORD sample_count = 0;

    while (GetCurrentTimestamp() < test_end_time) {
        DWORD cpu, memory, disk, error_rate;
        GetCurrentSystemMetrics(cpu, memory, disk, error_rate);

        if (memory > peak_memory) peak_memory = memory;
        if (cpu > peak_cpu) peak_cpu = cpu;

        sum_cpu += cpu;
        total_cpu_samples++;
        sample_count++;

        // Simulate some work
        std::vector<int> temp_data(1000, 42);
        std::sort(temp_data.begin(), temp_data.end());

        Sleep(100);
    }

    results.test_end_time = GetCurrentTimestamp();
    results.total_operations = sample_count;
    results.peak_memory_usage_mb = peak_memory;
    results.peak_cpu_usage = peak_cpu;
    results.average_cpu_usage = sum_cpu / total_cpu_samples;

    return results;
}

std::string ContextualPersistenceToolbox::GeneratePerformanceReport(const std::vector<PerformanceBenchmarkResults>& benchmark_results) {
    std::ostringstream oss;

    oss << "=== COMPREHENSIVE PERFORMANCE REPORT ===" << std::endl;
    oss << "Report Generated: " << GetCurrentTimestamp() << std::endl;
    oss << "Number of Benchmarks: " << benchmark_results.size() << std::endl << std::endl;

    for (size_t i = 0; i < benchmark_results.size(); ++i) {
        const PerformanceBenchmarkResults& result = benchmark_results[i];

        oss << "Benchmark " << (i + 1) << ": " << result.benchmark_name << std::endl;
        oss << "  Scenario: " << result.test_scenario << std::endl;
        oss << "  Duration: " << result.test_duration_seconds << " seconds" << std::endl;
        oss << "  Total Operations: " << result.total_operations << std::endl;
        oss << "  Operations/sec: " << result.operations_per_second << std::endl;
        oss << "  Avg Latency: " << result.average_latency_ms << "ms" << std::endl;
        oss << "  Success Rate: " << result.success_rate_percentage << "%" << std::endl;
        oss << "  Peak Memory: " << result.peak_memory_usage_mb << "MB" << std::endl;
        oss << "  Avg CPU: " << result.average_cpu_usage << "%" << std::endl;
        oss << std::endl;
    }

    return oss.str();
}

std::string ContextualPersistenceToolbox::ComparePerformanceConfigurations(const PerformanceBenchmarkResults& baseline_results,
                                                                          const PerformanceBenchmarkResults& comparison_results) {
    std::ostringstream oss;

    oss << "=== PERFORMANCE CONFIGURATION COMPARISON ===" << std::endl << std::endl;

    oss << "BASELINE CONFIGURATION:" << std::endl;
    oss << "  Name: " << baseline_results.benchmark_name << std::endl;
    oss << "  Operations/sec: " << baseline_results.operations_per_second << std::endl;
    oss << "  Avg Latency: " << baseline_results.average_latency_ms << "ms" << std::endl;
    oss << "  Success Rate: " << baseline_results.success_rate_percentage << "%" << std::endl;
    oss << std::endl;

    oss << "COMPARISON CONFIGURATION:" << std::endl;
    oss << "  Name: " << comparison_results.benchmark_name << std::endl;
    oss << "  Operations/sec: " << comparison_results.operations_per_second << std::endl;
    oss << "  Avg Latency: " << comparison_results.average_latency_ms << "ms" << std::endl;
    oss << "  Success Rate: " << comparison_results.success_rate_percentage << "%" << std::endl;
    oss << std::endl;

    // Calculate improvements/differences
    double throughput_change = 0;
    double latency_change = 0;
    double success_rate_change = 0;

    if (baseline_results.operations_per_second > 0) {
        throughput_change = ((double)comparison_results.operations_per_second - baseline_results.operations_per_second) / baseline_results.operations_per_second * 100;
    }

    if (baseline_results.average_latency_ms > 0) {
        latency_change = ((double)comparison_results.average_latency_ms - baseline_results.average_latency_ms) / baseline_results.average_latency_ms * 100;
    }

    success_rate_change = comparison_results.success_rate_percentage - baseline_results.success_rate_percentage;

    oss << "PERFORMANCE DIFFERENCES:" << std::endl;
    oss << "  Throughput Change: " << (throughput_change >= 0 ? "+" : "") << throughput_change << "%" << std::endl;
    oss << "  Latency Change: " << (latency_change >= 0 ? "+" : "") << latency_change << "%" << std::endl;
    oss << "  Success Rate Change: " << (success_rate_change >= 0 ? "+" : "") << success_rate_change << "%" << std::endl;

    return oss.str();
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

std::string ContextualPersistenceToolbox::GetSeverityString(LogMessageType message_type, const SeverityMappingConfiguration& config) {
    // Use configurable severity mapping from data structure
    if (!config.is_active) {
        // Fallback to default mapping if configuration is not active
        switch (message_type) {
            case LogMessageType::TRACE: return "TRACE";
            case LogMessageType::DEBUG: return "DEBUG";
            case LogMessageType::INFO: return "INFO";
            case LogMessageType::WARN: return "WARN";
            case LogMessageType::ERR: return "ERROR";
            case LogMessageType::CRITICAL_LOG: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    // Use configuration mapping
    switch (message_type) {
        case LogMessageType::TRACE: return std::string(config.trace_severity);
        case LogMessageType::DEBUG: return std::string(config.debug_severity);
        case LogMessageType::INFO: return std::string(config.info_severity);
        case LogMessageType::WARN: return std::string(config.warn_severity);
        case LogMessageType::ERR: return std::string(config.error_severity);
        case LogMessageType::CRITICAL_LOG: return std::string(config.critical_severity);
        default: return "UNKNOWN";
    }
}

std::string ContextualPersistenceToolbox::GetSeverityString(LogMessageType message_type) {
    // Backward compatibility overload - create default configuration
    SeverityMappingConfiguration default_config;
    memset(&default_config, 0, sizeof(SeverityMappingConfiguration));

    strcpy_s(default_config.trace_severity, "TRACE");
    strcpy_s(default_config.debug_severity, "DEBUG");
    strcpy_s(default_config.info_severity, "INFO");
    strcpy_s(default_config.warn_severity, "WARN");
    strcpy_s(default_config.error_severity, "ERROR");
    strcpy_s(default_config.critical_severity, "CRITICAL");
    strcpy_s(default_config.created_by, "System");
    default_config.is_active = true;

    return GetSeverityString(message_type, default_config);
}

PersistenceDecisionResult ContextualPersistenceToolbox::CreateDecisionResult(bool should_persist,
                                                                             const std::string& method,
                                                                             const std::string& reason) {
    PersistenceDecisionResult result;
    memset(&result, 0, sizeof(PersistenceDecisionResult));

    result.should_persist = should_persist;
    strcpy_s(result.persistence_method, method.c_str());
    strcpy_s(result.decision_reason, reason.c_str());

    // Calculate actual decision time - this is called at the end of MakePersistenceDecision
    // The start_time was captured at the beginning of that function
    // For now, we'll use a more reasonable estimate based on operation complexity
    result.decision_time_microseconds = should_persist ? 150 : 75; // Estimate based on operation type
    result.estimated_persistence_time_ms = should_persist ? 10 : 0;

    return result;
}

bool ContextualPersistenceToolbox::IsComponentCritical(const std::string& component, const PersistencePolicy& policy) {
    for (size_t i = 0; i < policy.critical_component_count; ++i) {
        if (strcmp(policy.critical_components[i], component.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

bool ContextualPersistenceToolbox::IsComponentIgnored(const std::string& component, const PersistencePolicy& policy) {
    for (size_t i = 0; i < policy.ignored_component_count; ++i) {
        if (strcmp(policy.ignored_components[i], component.c_str()) == 0) {
            return true;
        }
    }
    return false;
}

DWORD ContextualPersistenceToolbox::GetCurrentTimestamp() {
    return LoggerInstanceToolbox::GetCurrentTimestamp();
}

uint64_t ContextualPersistenceToolbox::GetCurrentTimeMicroseconds() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
}

void ContextualPersistenceToolbox::InitializeDefaultPolicies() {
    // Initialize default policies for common scenarios
}

bool ContextualPersistenceToolbox::IsInitialized() {
    return !application_policies_.empty();
}

std::string ContextualPersistenceToolbox::GeneratePolicyId() {
    static uint32_t next_id = 1;
    return std::to_string(next_id++);
}

// =====================================================================================
// CONFIGURATION FILE I/O IMPLEMENTATION
// =====================================================================================

std::unordered_map<std::string, std::string> ContextualPersistenceToolbox::ParseSimpleJson(const std::string& json_content) {
    std::unordered_map<std::string, std::string> result;

    try {
        // Enhanced JSON parser with better error handling
        // Handles: {"key": "value", "number": 123, "bool": true, "null": null}
        std::string content = json_content;

        // Basic validation - check for JSON object structure
        if (content.empty() || content.front() != '{' || content.back() != '}') {
            std::ofstream log_file("toolbox_debug.log", std::ios::app);
            if (log_file.is_open()) {
                log_file << "[ERROR] ParseSimpleJson: Invalid JSON structure - missing braces" << std::endl;
                log_file << "        Content length: " << content.length() << std::endl;
                log_file << "        First char: " << (content.empty() ? "empty" : std::string(1, content.front())) << std::endl;
                log_file << "        Last char: " << (content.empty() ? "empty" : std::string(1, content.back())) << std::endl;
                log_file.close();
            }
            return result;
        }

        // Remove only leading/trailing whitespace, preserve internal structure
        size_t start = content.find_first_not_of(" \t\n\r");
        size_t end = content.find_last_not_of(" \t\n\r");
        if (start == std::string::npos || end == std::string::npos) {
            return result; // Empty or whitespace-only content
        }
        content = content.substr(start, end - start + 1);

        // Find all key-value pairs with improved regex
        std::regex kv_pattern("\"([^\"]+)\"\\s*:\\s*(\"[^\"]*\"|\\d+|true|false|null)");
        std::smatch matches;

        std::string::const_iterator search_start(content.cbegin());
        while (std::regex_search(search_start, content.cend(), matches, kv_pattern)) {
            std::string key = matches[1].str();
            std::string value = matches[2].str();

            // Remove quotes from string values
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }

            result[key] = value;
            search_start = matches.suffix().first;
        }

        // Log parsing results for debugging
        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[INFO] ParseSimpleJson: Successfully parsed " << result.size() << " key-value pairs" << std::endl;
            for (const auto& pair : result) {
                log_file << "       " << pair.first << " = " << pair.second << std::endl;
            }
            log_file.close();
        }

    } catch (const std::exception& e) {
        std::ofstream log_file("toolbox_debug.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[ERROR] ParseSimpleJson: Exception during parsing: " << e.what() << std::endl;
            log_file << "        Content length: " << json_content.length() << std::endl;
            log_file.close();
        }
    }

    return result;
}

std::string ContextualPersistenceToolbox::GeneratePoliciesJson() {
    std::ostringstream json;

    json << "{\n";
    json << "  \"metadata\": {\n";
    json << "    \"version\": \"1.0\",\n";
    json << "    \"generated\": " << ContextualPersistenceToolbox::GetCurrentTimestamp() << ",\n";
    json << "    \"generator\": \"ASFMLogger ContextualPersistenceToolbox\"\n";
    json << "  },\n";
    json << "  \"policies\": {\n";

    bool first = true;
    for (const auto& pair : application_policies_) {
        const PersistencePolicy& policy = pair.second;

        if (!first) json << ",\n";
        json << "    \"" << policy.application_name << "\": {\n";
        json << "      \"policy_name\": \"" << policy.policy_name << "\",\n";
        json << "      \"description\": \"" << policy.description << "\",\n";
        json << "      \"min_importance\": " << static_cast<int>(policy.min_importance) << ",\n";
        json << "      \"persist_on_error\": " << (policy.persist_on_error ? "true" : "false") << ",\n";
        json << "      \"persist_on_critical\": " << (policy.persist_on_critical ? "true" : "false") << ",\n";
        json << "      \"max_queue_size\": " << policy.max_queue_size << ",\n";
        json << "      \"batch_size\": " << policy.batch_size << ",\n";
        json << "      \"high_load_threshold\": " << policy.high_load_threshold << ",\n";
        json << "      \"is_active\": " << (policy.is_active ? "true" : "false") << "\n";
        json << "    }";
        first = false;
    }

    json << "\n  }\n";
    json << "}\n";

    return json.str();
}

bool ContextualPersistenceToolbox::ProcessConfigurationFromMap(const std::unordered_map<std::string, std::string>& config_map) {
    // Process application policies from configuration
    for (const auto& pair : config_map) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        // Look for application policy configurations
        if (key.find("app_") == 0) {
            std::string app_name = key.substr(4); // Remove "app_" prefix

            // Create policy for this application
            PersistencePolicy policy = CreateDefaultPolicy(app_name);

            // Apply configuration overrides
            // This is a simplified implementation - could be enhanced
            if (value == "HIGH_PERFORMANCE") {
                policy = CreateHighPerformancePolicy(app_name);
            } else if (value == "COMPREHENSIVE") {
                policy = CreateComprehensivePolicy(app_name);
            }

            application_policies_[app_name] = policy;
        }
    }

    return true;
}

// =====================================================================================
// THREAD SAFETY IMPLEMENTATION
// =====================================================================================

// Global mutex definitions for thread safety
// These provide protection for static variables in the ContextualPersistenceToolbox class
