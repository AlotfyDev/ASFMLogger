/**
 * ASFMLogger LoggerInstance Implementation
 *
 * Stateful wrapper implementation for logger instance management.
 * Provides thread-safe object-oriented interface using LoggerInstanceToolbox internally.
 */

#include "stateful/LoggerInstance.hpp"
#include "toolbox/LoggerInstanceToolbox.hpp"
#include "toolbox/LoggingToolbox.hpp"
#include "structs/LoggerInstanceData.hpp"
#include <sstream>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <fstream>
#include <iostream>

// =====================================================================================
// CONSTRUCTORS AND DESTRUCTOR
// =====================================================================================

LoggerInstance::LoggerInstance()
    : data_(), instance_mutex_() {
    // Initialize with default instance data
    data_ = LoggerInstanceToolbox::CreateInstance("DefaultApplication", "DefaultProcess", "DefaultInstance");

    // Validate creation - throw exception if invalid
    if (!LoggerInstanceToolbox::ValidateInstance(data_)) {
        throw std::runtime_error("Failed to create valid default logger instance");
    }
}

LoggerInstance::LoggerInstance(const std::string& application_name,
                               const std::string& process_name,
                               const std::string& instance_name)
    : data_(), instance_mutex_() {

    // Validate inputs
    if (application_name.empty()) {
        throw std::invalid_argument("Application name cannot be empty");
    }

    // Use ToolBox to create instance data
    data_ = LoggerInstanceToolbox::CreateInstance(application_name, process_name, instance_name);

    // Validate creation
    if (!LoggerInstanceToolbox::ValidateInstance(data_)) {
        throw std::runtime_error("Failed to create valid logger instance");
    }
}

LoggerInstance::LoggerInstance(const LoggerInstanceData& data)
    : data_(data), instance_mutex_() {

    // Validate provided data
    if (!LoggerInstanceToolbox::ValidateInstance(data_)) {
        // Use LoggingToolbox instead of direct file I/O (fixed architecture violation)
        LoggingToolbox::Error("LoggerInstance: Invalid LoggerInstanceData provided",
                             "LoggerInstance", "Constructor");
        LoggingToolbox::Error("Instance ID: " + std::to_string(data.instance_id),
                             "LoggerInstance", "Constructor");
        LoggingToolbox::Error("Application: " + LoggerInstanceToolbox::GetApplicationName(data_),
                             "LoggerInstance", "Constructor");
        throw std::invalid_argument("Invalid LoggerInstanceData provided");
    }
}

LoggerInstance::LoggerInstance(const LoggerInstance& other)
    : data_(other.data_), instance_mutex_() {
    // Copy constructor with proper mutex handling
    std::lock_guard<std::mutex> lock(other.instance_mutex_);
    data_ = other.data_;
}

LoggerInstance::LoggerInstance(LoggerInstance&& other) noexcept
    : data_(other.data_), instance_mutex_() {
    // Move constructor - data is already moved
}

LoggerInstance& LoggerInstance::operator=(const LoggerInstance& other) {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(instance_mutex_);
        std::lock_guard<std::mutex> other_lock(other.instance_mutex_);

        data_ = other.data_;
    }
    return *this;
}

LoggerInstance& LoggerInstance::operator=(LoggerInstance&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(instance_mutex_);

        data_ = other.data_;
        // other.data_ is left in valid but unspecified state
    }
    return *this;
}

// =====================================================================================
// FACTORY METHODS
// =====================================================================================

LoggerInstance LoggerInstance::Create(const std::string& application_name,
                                     const std::string& process_name,
                                     const std::string& instance_name) {
    return LoggerInstance(application_name, process_name, instance_name);
}

LoggerInstance LoggerInstance::clone() const {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    return LoggerInstance(data_);
}

// =====================================================================================
// GETTERS (Thread-safe using toolbox internally)
// =====================================================================================

// Getters are already defined inline in the header file

// =====================================================================================
// SETTERS (Thread-safe using toolbox internally)
// =====================================================================================

bool LoggerInstance::setApplicationInfo(const std::string& application_name, const std::string& process_name) {
    std::lock_guard<std::mutex> lock(instance_mutex_);

    // Validate inputs
    if (application_name.empty()) {
        // Use LoggingToolbox instead of direct file I/O (fixed architecture violation)
        LoggingToolbox::Error("LoggerInstance::setApplicationInfo: Application name cannot be empty",
                             "LoggerInstance", "setApplicationInfo");
        return false;
    }

    // Use ToolBox for actual operation
    bool success = LoggerInstanceToolbox::SetApplicationInfo(data_, application_name, process_name);

    // Update activity timestamp on successful change
    if (success) {
        LoggerInstanceToolbox::UpdateActivity(data_);

        // Use LoggingToolbox instead of direct file I/O (fixed architecture violation)
        LoggingToolbox::Info("LoggerInstance::setApplicationInfo: Updated successfully",
                            "LoggerInstance", "setApplicationInfo");
        LoggingToolbox::Info("Application: " + application_name,
                            "LoggerInstance", "setApplicationInfo");
        LoggingToolbox::Info("Process: " + (process_name.empty() ? "Default" : process_name),
                            "LoggerInstance", "setApplicationInfo");
    }

    return success;
}

bool LoggerInstance::setInstanceName(const std::string& instance_name) {
    std::lock_guard<std::mutex> lock(instance_mutex_);

    // Validate input
    if (instance_name.empty()) {
        // Use LoggingToolbox instead of direct file I/O (fixed architecture violation)
        LoggingToolbox::Error("LoggerInstance::setInstanceName: Instance name cannot be empty",
                             "LoggerInstance", "setInstanceName");
        return false;
    }

    // Use ToolBox for actual operation
    bool success = LoggerInstanceToolbox::SetInstanceName(data_, instance_name);

    // Update activity timestamp on successful change
    if (success) {
        LoggerInstanceToolbox::UpdateActivity(data_);

        // Use LoggingToolbox instead of direct file I/O (fixed architecture violation)
        LoggingToolbox::Info("LoggerInstance::setInstanceName: Updated successfully",
                            "LoggerInstance", "setInstanceName");
        LoggingToolbox::Info("Instance: " + instance_name,
                            "LoggerInstance", "setInstanceName");
    }

    return success;
}

// =====================================================================================
// ACTIVITY TRACKING (Thread-safe)
// =====================================================================================

// Activity tracking functions are already defined inline in the header file

// =====================================================================================
// VALIDATION AND INSPECTION (Thread-safe)
// =====================================================================================

// Validation and inspection functions are already defined inline in the header file

// =====================================================================================
// COMPARISON OPERATORS IMPLEMENTATION
// =====================================================================================

bool LoggerInstance::operator<(const LoggerInstance& other) const {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    std::lock_guard<std::mutex> other_lock(other.instance_mutex_);
    DWORD this_time = LoggerInstanceToolbox::GetCreationTime(data_);
    DWORD other_time = LoggerInstanceToolbox::GetCreationTime(other.data_);
    return this_time < other_time;
}

bool LoggerInstance::operator>(const LoggerInstance& other) const {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    std::lock_guard<std::mutex> other_lock(other.instance_mutex_);
    DWORD this_time = LoggerInstanceToolbox::GetCreationTime(data_);
    DWORD other_time = LoggerInstanceToolbox::GetCreationTime(other.data_);
    return this_time > other_time;
}

// =====================================================================================
// UTILITY METHODS
// =====================================================================================

void LoggerInstance::swap(LoggerInstance& other) noexcept {
    std::lock_guard<std::mutex> this_lock(instance_mutex_);
    std::lock_guard<std::mutex> other_lock(other.instance_mutex_);

    std::swap(data_, other.data_);
}

void LoggerInstance::clear() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    // Reset instance to default state using toolbox
    data_ = LoggerInstanceToolbox::CreateInstance("DefaultApplication", "DefaultProcess", "DefaultInstance");
}

DWORD LoggerInstance::getIdleTime() const {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    DWORD current_time = LoggerInstanceToolbox::GetCurrentTimestamp();
    DWORD last_activity = LoggerInstanceToolbox::GetLastActivity(data_);
    return (current_time >= last_activity) ? current_time - last_activity : 0;
}


LoggerInstance CreateApplicationInstance(const std::string& application_name,
                                        const std::string& process_name,
                                        const std::string& instance_name) {
    return LoggerInstance::Create(application_name, process_name, instance_name);
}

LoggerInstance CreateDefaultInstance() {
    return LoggerInstance();
}

std::vector<LoggerInstance> CreateMultipleInstances(const std::vector<std::string>& application_names,
                                                   const std::string& process_name,
                                                   const std::string& base_instance_name) {
    std::vector<LoggerInstance> instances;

    for (size_t i = 0; i < application_names.size(); ++i) {
        std::string instance_name = base_instance_name + "_" + std::to_string(i + 1);
        instances.emplace_back(application_names[i], process_name, instance_name);
    }

    return instances;
}

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

void swap(LoggerInstance& a, LoggerInstance& b) noexcept {
    a.swap(b);
}

std::ostream& operator<<(std::ostream& os, const LoggerInstance& instance) {
    os << instance.toString();
    return os;
}

LoggerInstance CreateCurrentApplicationInstance(const std::string& application_name,
                                               const std::string& process_name) {
    // Get current process information using std::tie to avoid structured bindings
    DWORD process_id;
    std::string process_name_current;

    auto process_info = LoggerInstanceToolbox::GetCurrentProcessInfo();
    std::tie(process_id, process_name_current) = process_info;

    std::string final_process_name = process_name;
    if (final_process_name.empty()) {
        final_process_name = process_name_current;
    }

    return LoggerInstance::Create(application_name, final_process_name, "CurrentInstance");
}
