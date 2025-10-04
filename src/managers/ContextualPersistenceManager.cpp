/**
 * ASFMLogger ContextualPersistenceManager Implementation
 *
 * Stateful manager class for contextual persistence policy management.
 * Uses ContextualPersistenceToolbox internally for all decision logic.
 */

#include "ContextualPersistenceManager.hpp"
#include "toolbox/ContextualPersistenceToolbox.hpp"
#include <sstream>
#include <chrono>
#include <algorithm>

#pragma comment(lib, "Pdh.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "odbc32.lib")

// =====================================================================================
// CONSTRUCTORS AND DESTRUCTOR
// =====================================================================================

ContextualPersistenceManager::ContextualPersistenceManager()
    : application_name_("default"),
      importance_mapper_(nullptr),
      emergency_mode_active_(false),
      emergency_mode_end_time_(0),
      total_decisions_made_(0),
      total_messages_persisted_(0),
      total_messages_dropped_(0) {
    // Initialize with default policy
    initializeWithDefaults();
}

ContextualPersistenceManager::ContextualPersistenceManager(const std::string& application_name)
    : application_name_(application_name),
      importance_mapper_(nullptr),
      emergency_mode_active_(false),
      emergency_mode_end_time_(0),
      total_decisions_made_(0),
      total_messages_persisted_(0),
      total_messages_dropped_(0) {
    // Validate application name
    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Initialize with default policy for this application
    initializeWithDefaults();
}

ContextualPersistenceManager::ContextualPersistenceManager(const std::string& application_name,
                                                         const PersistencePolicy& policy)
    : application_name_(application_name),
      current_policy_(policy),
      importance_mapper_(nullptr),
      emergency_mode_active_(false),
      emergency_mode_end_time_(0),
      total_decisions_made_(0),
      total_messages_persisted_(0),
      total_messages_dropped_(0) {
    // Validate application name
    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Set policy in toolbox
    ContextualPersistenceToolbox::SetApplicationPolicy(application_name, policy);
}

ContextualPersistenceManager::ContextualPersistenceManager(const std::string& application_name,
                                                         ImportanceMapper* importance_mapper)
    : application_name_(application_name),
      importance_mapper_(importance_mapper),
      emergency_mode_active_(false),
      emergency_mode_end_time_(0),
      total_decisions_made_(0),
      total_messages_persisted_(0),
      total_messages_dropped_(0) {
    // Validate inputs
    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Important: Set the ImportanceMapper for decision coordination
    if (importance_mapper != nullptr) {
        importance_mapper_ = importance_mapper;
    }

    // Initialize with default policy for this application
    initializeWithDefaults();
}

ContextualPersistenceManager::ContextualPersistenceManager(const ContextualPersistenceManager& other)
    : application_name_(other.application_name_),
      current_policy_(other.current_policy_),
      importance_mapper_(other.importance_mapper_),
      emergency_mode_active_(other.emergency_mode_active_),
      emergency_mode_end_time_(other.emergency_mode_end_time_),
      total_decisions_made_(other.total_decisions_made_),
      total_messages_persisted_(other.total_messages_persisted_),
      total_messages_dropped_(other.total_messages_dropped_) {
    // No additional initialization needed
}

ContextualPersistenceManager::ContextualPersistenceManager(ContextualPersistenceManager&& other) noexcept
    : application_name_(std::move(other.application_name_)),
      current_policy_(std::move(other.current_policy_)),
      importance_mapper_(other.importance_mapper_),
      emergency_mode_active_(other.emergency_mode_active_),
      emergency_mode_end_time_(other.emergency_mode_end_time_),
      total_decisions_made_(other.total_decisions_made_),
      total_messages_persisted_(other.total_messages_persisted_),
      total_messages_dropped_(other.total_messages_dropped_) {
    // Clear moved-from state
    other.application_name_.clear();
    other.importance_mapper_ = nullptr;
    other.emergency_mode_active_ = false;
    other.emergency_mode_end_time_ = 0;
    other.total_decisions_made_ = 0;
    other.total_messages_persisted_ = 0;
    other.total_messages_dropped_ = 0;
}

ContextualPersistenceManager& ContextualPersistenceManager::operator=(const ContextualPersistenceManager& other) {
    if (this != &other) {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        application_name_ = other.application_name_;
        current_policy_ = other.current_policy_;
        importance_mapper_ = other.importance_mapper_;
        emergency_mode_active_ = other.emergency_mode_active_;
        emergency_mode_end_time_ = other.emergency_mode_end_time_;
        total_decisions_made_ = other.total_decisions_made_;
        total_messages_persisted_ = other.total_messages_persisted_;
        total_messages_dropped_ = other.total_messages_dropped_;
        last_policy_evaluation_ = other.last_policy_evaluation_;
    }
    return *this;
}

ContextualPersistenceManager& ContextualPersistenceManager::operator=(ContextualPersistenceManager&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        application_name_ = std::move(other.application_name_);
        current_policy_ = std::move(other.current_policy_);
        importance_mapper_ = other.importance_mapper_;
        emergency_mode_active_ = other.emergency_mode_active_;
        emergency_mode_end_time_ = other.emergency_mode_end_time_;
        total_decisions_made_ = other.total_decisions_made_;
        total_messages_persisted_ = other.total_messages_persisted_;
        total_messages_dropped_ = other.total_messages_dropped_;
        last_policy_evaluation_ = other.last_policy_evaluation_;

        // Clear moved-from state
        other.application_name_.clear();
        other.importance_mapper_ = nullptr;
        other.emergency_mode_active_ = false;
        other.emergency_mode_end_time_ = 0;
        other.total_decisions_made_ = 0;
        other.total_messages_persisted_ = 0;
        other.total_messages_dropped_ = 0;
    }
    return *this;
}

ContextualPersistenceManager::~ContextualPersistenceManager() {
    // Cleanup if needed
}

// =====================================================================================
// APPLICATION MANAGEMENT
// =====================================================================================

void ContextualPersistenceManager::setApplicationName(const std::string& application_name) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    application_name_ = application_name;

    // Re-initialize with new application context
    initializeWithDefaults();
}

void ContextualPersistenceManager::setImportanceMapper(ImportanceMapper* importance_mapper) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    importance_mapper_ = importance_mapper;
}

// =====================================================================================
// POLICY MANAGEMENT
// =====================================================================================

bool ContextualPersistenceManager::setPolicy(const PersistencePolicy& policy) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Validate policy
    if (!ContextualPersistenceToolbox::ValidatePolicy(policy)) {
        return false;
    }

    // Set policy in toolbox
    if (!ContextualPersistenceToolbox::SetApplicationPolicy(application_name_, policy)) {
        return false;
    }

    current_policy_ = policy;
    return true;
}

bool ContextualPersistenceManager::loadPolicy(const PersistencePolicy& config) {
    // Delegate to setPolicy for validation and setting
    return setPolicy(config);
}

bool ContextualPersistenceManager::resetToDefaults() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Reset policy to defaults in toolbox
    ContextualPersistenceToolbox::ResetPoliciesToDefaults();

    // Create new default policy
    initializeWithDefaults();
    return true;
}

bool ContextualPersistenceManager::createHighPerformancePolicy() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Create high-performance policy using toolbox
    PersistencePolicy policy = ContextualPersistenceToolbox::CreateHighPerformancePolicy(application_name_);

    return setPolicy(policy);
}

bool ContextualPersistenceManager::createComprehensivePolicy() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Create comprehensive policy using toolbox
    PersistencePolicy policy = ContextualPersistenceToolbox::CreateComprehensivePolicy(application_name_);

    return setPolicy(policy);
}

bool ContextualPersistenceManager::createBalancedPolicy() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Create default balanced policy using toolbox
    PersistencePolicy policy = ContextualPersistenceToolbox::CreateDefaultPolicy(application_name_);

    return setPolicy(policy);
}

// =====================================================================================
// PERSISTENCE DECISIONS - KEY INTEGRATION POINT
// =====================================================================================

PersistenceDecisionResult ContextualPersistenceManager::makePersistenceDecision(const LogMessageData& message) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    total_decisions_made_++;

    // Evaluate current context
    PersistenceDecisionContext context = evaluateCurrentContext();

    // Use ContextualPersistenceToolbox to make the actual decision
    // This is the key integration point: Stateful layer delegates to Toolbox layer
    PersistenceDecisionResult result = ContextualPersistenceToolbox::MakePersistenceDecision(
        message, context, current_policy_);

    // Update statistics
    updateInternalStatistics(result, result.estimated_persistence_time_ms);

    return result;
}

bool ContextualPersistenceManager::shouldPersistQuick(LogMessageType message_type,
                                                    MessageImportance resolved_importance) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    total_decisions_made_++;

    // Evaluate current context (lightweight version)
    PersistenceDecisionContext context = createCurrentContext();

    // Get adjusted importance from system load
    DWORD system_load = context.current_system_load;

    // Use Toolbox for quick decision
    bool should_persist = ContextualPersistenceToolbox::ShouldPersistQuick(
        message_type, resolved_importance, system_load, current_policy_);

    // Update statistics (minimal)
    if (should_persist) {
        total_messages_persisted_++;
    } else {
        total_messages_dropped_++;
    }

    return should_persist;
}

// =====================================================================================
// EMERGENCY MODE MANAGEMENT
// =====================================================================================

bool ContextualPersistenceManager::enterEmergencyMode(DWORD duration_seconds) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Use Toolbox for emergency mode management
    bool success = ContextualPersistenceToolbox::EnterEmergencyMode(application_name_, duration_seconds);

    if (success) {
        emergency_mode_active_ = true;
        emergency_mode_end_time_ = GetCurrentTimestamp() + duration_seconds;
    }

    return success;
}

bool ContextualPersistenceManager::exitEmergencyMode() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Use Toolbox for emergency mode management
    bool success = ContextualPersistenceToolbox::ExitEmergencyMode(application_name_);

    if (success) {
        emergency_mode_active_ = false;
        emergency_mode_end_time_ = 0;
    }

    return success;
}

DWORD ContextualPersistenceManager::getEmergencyModeTimeRemaining() const {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Use Toolbox to get remaining time
    return ContextualPersistenceToolbox::GetEmergencyModeTimeRemaining(application_name_);
}

bool ContextualPersistenceManager::extendEmergencyMode(DWORD additional_seconds) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    if (!emergency_mode_active_) {
        return false;
    }

    emergency_mode_end_time_ += additional_seconds;

    // Also extend in Toolbox
    return enterEmergencyMode(additional_seconds);
}

// =====================================================================================
// STATISTICS AND MONITORING
// =====================================================================================

PersistenceStatistics ContextualPersistenceManager::getStatistics() const {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Get statistics from Toolbox
    return ContextualPersistenceToolbox::GetPersistenceStatistics(application_name_);
}

void ContextualPersistenceManager::resetStatistics() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    total_decisions_made_ = 0;
    total_messages_persisted_ = 0;
    total_messages_dropped_ = 0;

    // Reset in Toolbox as well
    ContextualPersistenceToolbox::ResetPersistenceStatistics(application_name_);
}

void ContextualPersistenceManager::updateStatistics(const PersistenceDecisionResult& decision,
                                                  DWORD actual_persistence_time_ms) {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Update Toolbox statistics
    ContextualPersistenceToolbox::UpdatePersistenceStatistics(
        application_name_, decision, actual_persistence_time_ms);
}

// =====================================================================================
// ADAPTIVE POLICY MANAGEMENT
// =====================================================================================

bool ContextualPersistenceManager::evaluateAndAdaptPolicy() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Check if evaluation is needed
    if (!shouldEvaluatePolicy()) {
        return false;
    }

    // Evaluate current context for adaptive triggers
    PersistenceDecisionContext context = evaluateCurrentContext();

    // Check for adaptive triggers
    auto adapted_policies = ContextualPersistenceToolbox::CheckAdaptiveTriggers(
        application_name_, context);

    if (!adapted_policies.empty()) {
        // Use the first adapted policy
        current_policy_ = adapted_policies[0];
        last_policy_evaluation_ = std::chrono::steady_clock::now();
        return true;
    }

    last_policy_evaluation_ = std::chrono::steady_clock::now();
    return false;
}

bool ContextualPersistenceManager::forcePolicyEvaluation() {
    std::lock_guard<std::mutex> lock(manager_mutex_);

    // Evaluate current context for adaptive triggers
    PersistenceDecisionContext context = evaluateCurrentContext();

    // Check for adaptive triggers
    auto adapted_policies = ContextualPersistenceToolbox::CheckAdaptiveTriggers(
        application_name_, context);

    if (!adapted_policies.empty()) {
        // Use the first adapted policy
        current_policy_ = adapted_policies[0];
        return true;
    }

    return false;
}

// =====================================================================================
// CONTEXT EVALUATION
// =====================================================================================

PersistenceDecisionContext ContextualPersistenceManager::evaluateCurrentContext() const {
    // Use Toolbox for context evaluation
    return ContextualPersistenceToolbox::EvaluateSystemContext(application_name_);
}

bool ContextualPersistenceManager::isHighLoad() const {
    PersistenceDecisionContext context = evaluateCurrentContext();
    return ContextualPersistenceToolbox::IsHighLoad(context);
}

bool ContextualPersistenceManager::isElevatedErrorRate() const {
    PersistenceDecisionContext context = evaluateCurrentContext();
    return ContextualPersistenceToolbox::IsElevatedErrorRate(context);
}

bool ContextualPersistenceManager::isEmergencyCondition() const {
    PersistenceDecisionContext context = evaluateCurrentContext();
    return ContextualPersistenceToolbox::IsEmergencyCondition(context);
}

DWORD ContextualPersistenceManager::getSystemStressLevel() const {
    PersistenceDecisionContext context = evaluateCurrentContext();
    return ContextualPersistenceToolbox::CalculateSystemStressLevel(context);
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

void ContextualPersistenceManager::initializeWithDefaults() {
    // Create and set default policy using Toolbox
    current_policy_ = ContextualPersistenceToolbox::CreateDefaultPolicy(application_name_);
    ContextualPersistenceToolbox::SetApplicationPolicy(application_name_, current_policy_);

    // Initialize timing
    last_policy_evaluation_ = std::chrono::steady_clock::now();
}

void ContextualPersistenceManager::updateInternalStatistics(const PersistenceDecisionResult& decision,
                                                         DWORD persistence_time_ms) {
    if (decision.should_persist) {
        total_messages_persisted_++;
    } else {
        total_messages_dropped_++;
    }

    // Update Toolbox statistics
    ContextualPersistenceToolbox::UpdatePersistenceStatistics(
        application_name_, decision, persistence_time_ms);
}

PersistenceDecisionContext ContextualPersistenceManager::createCurrentContext() const {
    // Lightweight context creation for quick decisions
    return ContextualPersistenceToolbox::EvaluateSystemContext(application_name_);
}

bool ContextualPersistenceManager::shouldEvaluatePolicy() const {
    // Check if enough time has passed since last evaluation
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_eval = now - last_policy_evaluation_;

    // Evaluate at most every minute
    return time_since_last_eval > std::chrono::minutes(1);
}

DWORD ContextualPersistenceManager::GetCurrentTimestamp() const {
    // Use Windows API for timestamp
    return static_cast<DWORD>(GetTickCount());
}

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

// Static storage for global managers (thread-safe with mutex)
static std::unordered_map<std::string, ContextualPersistenceManager> global_persistence_managers_;
static std::mutex global_managers_mutex_;

ContextualPersistenceManager& GetGlobalPersistenceManager(const std::string& application_name) {
    std::lock_guard<std::mutex> lock(global_managers_mutex_);

    auto it = global_persistence_managers_.find(application_name);
    if (it == global_persistence_managers_.end()) {
        // Create new manager for this application
        auto result = global_persistence_managers_.emplace(application_name, application_name);
        return result.first->second;
    }

    return it->second;
}

ContextualPersistenceManager CreateApplicationPersistenceManager(const std::string& application_name) {
    return ContextualPersistenceManager(application_name);
}
