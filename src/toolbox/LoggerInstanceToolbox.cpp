/**
 * ASFMLogger Instance Management Toolbox Implementation
 *
 * Static methods for logger instance management following toolbox architecture.
 * Pure functions for instance lifecycle and coordination.
 */

#include "LoggerInstanceToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include "../structs/LoggerInstanceData.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <Windows.h>
#include <Lmcons.h>
#include <process.h>
#include <chrono>

// Static variables
std::atomic<uint32_t> LoggerInstanceToolbox::global_instance_id_counter_{1};
std::mutex LoggerInstanceToolbox::instance_management_mutex_;

// =====================================================================================
// INSTANCE ID GENERATION
// =====================================================================================

uint32_t LoggerInstanceToolbox::GenerateInstanceId() {
    return global_instance_id_counter_.fetch_add(1, std::memory_order_relaxed);
}

void LoggerInstanceToolbox::ResetInstanceIdCounter() {
    std::lock_guard<std::mutex> lock(instance_management_mutex_);
    global_instance_id_counter_.store(1, std::memory_order_relaxed);
}

// =====================================================================================
// INSTANCE CREATION
// =====================================================================================

LoggerInstanceData LoggerInstanceToolbox::CreateInstance(
    const std::string& application_name,
    const std::string& process_name,
    const std::string& instance_name) {

    LoggerInstanceData instance;
    memset(&instance, 0, sizeof(LoggerInstanceData));

    instance.instance_id = GenerateInstanceId();
    instance.creation_time = GetCurrentTimestamp();

    // Set application information
    SetApplicationInfo(instance, application_name, process_name);
    SetInstanceName(instance, instance_name.empty() ?
        GenerateInstanceName(application_name, process_name) : instance_name);

    // Initialize with system information
    InitializeInstance(instance);

    return instance;
}

bool LoggerInstanceToolbox::InitializeInstance(LoggerInstanceData& instance) {
    // Get current process information
    auto [process_id, process_name] = GetCurrentProcessInfo();
    instance.process_id = process_id;

    // Set system information
    std::string host_name = GetHostName();
    std::string user_name = GetCurrentUserName();

    SetStringField(instance.host_name, host_name, sizeof(instance.host_name));
    SetStringField(instance.user_name, user_name, sizeof(instance.user_name));

    // Set initial activity time
    instance.last_activity = GetCurrentTimestamp();

    return true;
}

bool LoggerInstanceToolbox::SetApplicationInfo(LoggerInstanceData& instance,
                                              const std::string& application_name,
                                              const std::string& process_name) {
    bool success = true;

    if (!SetStringField(instance.application_name, application_name, sizeof(instance.application_name))) {
        success = false;
    }

    if (!process_name.empty()) {
        if (!SetStringField(instance.process_name, process_name, sizeof(instance.process_name))) {
            success = false;
        }
    }

    return success;
}

bool LoggerInstanceToolbox::SetInstanceName(LoggerInstanceData& instance, const std::string& instance_name) {
    return SetStringField(instance.instance_name, instance_name, sizeof(instance.instance_name));
}

// =====================================================================================
// INSTANCE VALIDATION
// =====================================================================================

bool LoggerInstanceToolbox::ValidateInstance(const LoggerInstanceData& instance) {
    return instance.instance_id != 0 &&
           instance.process_id != 0 &&
           !IsStringEmpty(instance.application_name) &&
           instance.creation_time > 0;
}

bool LoggerInstanceToolbox::HasApplicationInfo(const LoggerInstanceData& instance) {
    return !IsStringEmpty(instance.application_name);
}

bool LoggerInstanceToolbox::IsInstanceActive(const LoggerInstanceData& instance, DWORD max_idle_seconds) {
    DWORD current_time = GetCurrentTimestamp();
    DWORD idle_time = current_time - instance.last_activity;
    return idle_time <= max_idle_seconds;
}

// =====================================================================================
// INSTANCE INFORMATION EXTRACTION
// =====================================================================================

uint32_t LoggerInstanceToolbox::GetInstanceId(const LoggerInstanceData& instance) {
    return instance.instance_id;
}

std::string LoggerInstanceToolbox::GetApplicationName(const LoggerInstanceData& instance) {
    return GetStringField(instance.application_name);
}

std::string LoggerInstanceToolbox::GetProcessName(const LoggerInstanceData& instance) {
    return GetStringField(instance.process_name);
}

std::string LoggerInstanceToolbox::GetInstanceName(const LoggerInstanceData& instance) {
    return GetStringField(instance.instance_name);
}

DWORD LoggerInstanceToolbox::GetProcessId(const LoggerInstanceData& instance) {
    return instance.process_id;
}

DWORD LoggerInstanceToolbox::GetCreationTime(const LoggerInstanceData& instance) {
    return instance.creation_time;
}

DWORD LoggerInstanceToolbox::GetLastActivity(const LoggerInstanceData& instance) {
    return instance.last_activity;
}

uint64_t LoggerInstanceToolbox::GetMessageCount(const LoggerInstanceData& instance) {
    return instance.message_count;
}

uint64_t LoggerInstanceToolbox::GetErrorCount(const LoggerInstanceData& instance) {
    return instance.error_count;
}

// =====================================================================================
// INSTANCE ACTIVITY TRACKING
// =====================================================================================

void LoggerInstanceToolbox::UpdateActivity(LoggerInstanceData& instance) {
    instance.last_activity = GetCurrentTimestamp();
}

void LoggerInstanceToolbox::IncrementMessageCount(LoggerInstanceData& instance) {
    instance.message_count++;
    UpdateActivity(instance);
}

void LoggerInstanceToolbox::IncrementErrorCount(LoggerInstanceData& instance) {
    instance.error_count++;
    UpdateActivity(instance);
}

void LoggerInstanceToolbox::UpdateStatistics(LoggerInstanceData& instance,
                                           uint64_t message_count,
                                           uint64_t error_count) {
    instance.message_count = message_count;
    instance.error_count = error_count;
    UpdateActivity(instance);
}

// =====================================================================================
// INSTANCE COMPARISON AND SEARCHING
// =====================================================================================

bool LoggerInstanceToolbox::CompareInstances(const LoggerInstanceData& a, const LoggerInstanceData& b) {
    return a.instance_id == b.instance_id &&
           a.process_id == b.process_id &&
           strcmp(a.application_name, b.application_name) == 0;
}

bool LoggerInstanceToolbox::AreSameApplication(const LoggerInstanceData& a, const LoggerInstanceData& b) {
    return strcmp(a.application_name, b.application_name) == 0;
}

bool LoggerInstanceToolbox::AreSameProcess(const LoggerInstanceData& a, const LoggerInstanceData& b) {
    return a.process_id == b.process_id &&
           strcmp(a.application_name, b.application_name) == 0;
}

std::vector<LoggerInstanceData>::iterator LoggerInstanceToolbox::FindInstanceById(
    std::vector<LoggerInstanceData>& instances,
    uint32_t instance_id) {

    return std::find_if(instances.begin(), instances.end(),
                       [instance_id](const LoggerInstanceData& instance) {
                           return instance.instance_id == instance_id;
                       });
}

std::vector<LoggerInstanceData> LoggerInstanceToolbox::FindInstancesByApplication(
    const std::vector<LoggerInstanceData>& instances,
    const std::string& application_name) {

    std::vector<LoggerInstanceData> result;
    std::string app_name_lower = application_name;
    std::transform(app_name_lower.begin(), app_name_lower.end(), app_name_lower.begin(), ::tolower);

    std::copy_if(instances.begin(), instances.end(), std::back_inserter(result),
                 [&](const LoggerInstanceData& instance) {
                     std::string instance_app = GetApplicationName(instance);
                     std::transform(instance_app.begin(), instance_app.end(), instance_app.begin(), ::tolower);
                     return instance_app.find(app_name_lower) != std::string::npos;
                 });

    return result;
}

// =====================================================================================
// INSTANCE COLLECTION MANAGEMENT
// =====================================================================================

std::vector<LoggerInstanceData> LoggerInstanceToolbox::FilterByApplication(
    const std::vector<LoggerInstanceData>& instances,
    const std::string& application_name) {

    std::vector<LoggerInstanceData> result;
    std::string app_name_lower = application_name;
    std::transform(app_name_lower.begin(), app_name_lower.end(), app_name_lower.begin(), ::tolower);

    std::copy_if(instances.begin(), instances.end(), std::back_inserter(result),
                 [&](const LoggerInstanceData& instance) {
                     std::string instance_app = GetApplicationName(instance);
                     std::transform(instance_app.begin(), instance_app.end(), instance_app.begin(), ::tolower);
                     return instance_app == app_name_lower;
                 });

    return result;
}

std::vector<LoggerInstanceData> LoggerInstanceToolbox::FilterActiveInstances(
    const std::vector<LoggerInstanceData>& instances,
    DWORD max_idle_seconds) {

    std::vector<LoggerInstanceData> result;
    DWORD current_time = GetCurrentTimestamp();

    std::copy_if(instances.begin(), instances.end(), std::back_inserter(result),
                 [&](const LoggerInstanceData& instance) {
                     DWORD idle_time = current_time - instance.last_activity;
                     return idle_time <= max_idle_seconds;
                 });

    return result;
}

void LoggerInstanceToolbox::SortByCreationTime(std::vector<LoggerInstanceData>& instances) {
    std::sort(instances.begin(), instances.end(),
              [](const LoggerInstanceData& a, const LoggerInstanceData& b) {
                  return a.creation_time < b.creation_time;
              });
}

void LoggerInstanceToolbox::SortByActivity(std::vector<LoggerInstanceData>& instances) {
    std::sort(instances.begin(), instances.end(),
              [](const LoggerInstanceData& a, const LoggerInstanceData& b) {
                  return a.last_activity > b.last_activity; // Most recent first
              });
}

size_t LoggerInstanceToolbox::RemoveInactiveInstances(std::vector<LoggerInstanceData>& instances,
                                                     DWORD max_idle_seconds) {
    DWORD current_time = GetCurrentTimestamp();
    size_t original_size = instances.size();

    instances.erase(
        std::remove_if(instances.begin(), instances.end(),
                      [&](const LoggerInstanceData& instance) {
                          DWORD idle_time = current_time - instance.last_activity;
                          return idle_time > max_idle_seconds;
                      }),
        instances.end());

    return original_size - instances.size();
}

// =====================================================================================
// INSTANCE STATISTICS
// =====================================================================================

InstanceStatistics LoggerInstanceToolbox::CalculateStatistics(const LoggerInstanceData& instance) {
    InstanceStatistics stats;
    memset(&stats, 0, sizeof(InstanceStatistics));

    stats.instance_id = instance.instance_id;
    stats.total_messages = instance.message_count;
    stats.total_errors = instance.error_count;
    stats.uptime_seconds = CalculateUptime(instance);

    // Calculate rates
    if (stats.uptime_seconds > 0) {
        stats.messages_per_second = static_cast<uint64_t>(CalculateMessageRate(instance, GetCurrentTimestamp()));
        stats.error_rate = static_cast<uint64_t>(CalculateErrorRate(instance, GetCurrentTimestamp()));
    }

    return stats;
}

DWORD LoggerInstanceToolbox::CalculateUptime(const LoggerInstanceData& instance) {
    DWORD current_time = GetCurrentTimestamp();
    if (current_time < instance.creation_time) {
        return 0;
    }
    return current_time - instance.creation_time;
}

double LoggerInstanceToolbox::CalculateMessageRate(const LoggerInstanceData& instance, DWORD current_time) {
    DWORD uptime = CalculateUptime(instance);
    if (uptime == 0) return 0.0;

    return static_cast<double>(instance.message_count) / static_cast<double>(uptime);
}

double LoggerInstanceToolbox::CalculateErrorRate(const LoggerInstanceData& instance, DWORD current_time) {
    DWORD uptime = CalculateUptime(instance);
    if (uptime == 0) return 0.0;

    return static_cast<double>(instance.error_count) / static_cast<double>(uptime);
}

std::vector<std::string> LoggerInstanceToolbox::GetUniqueApplications(
    const std::vector<LoggerInstanceData>& instances) {

    std::unordered_set<std::string> unique_apps;
    for (const auto& instance : instances) {
        std::string app_name = GetApplicationName(instance);
        if (!app_name.empty()) {
            unique_apps.insert(app_name);
        }
    }

    std::vector<std::string> result;
    for (const auto& app : unique_apps) {
        result.push_back(app);
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::pair<std::string, size_t>> LoggerInstanceToolbox::CountByApplication(
    const std::vector<LoggerInstanceData>& instances) {

    std::unordered_map<std::string, size_t> counts;
    for (const auto& instance : instances) {
        std::string app_name = GetApplicationName(instance);
        if (!app_name.empty()) {
            counts[app_name]++;
        }
    }

    std::vector<std::pair<std::string, size_t>> result;
    for (const auto& pair : counts) {
        result.emplace_back(pair.first, pair.second);
    }

    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return result;
}

// =====================================================================================
// INSTANCE FORMATTING
// =====================================================================================

std::string LoggerInstanceToolbox::InstanceToString(const LoggerInstanceData& instance) {
    std::ostringstream oss;
    oss << "[" << instance.instance_id << "] "
        << GetApplicationName(instance);

    if (!GetProcessName(instance).empty()) {
        oss << " (" << GetProcessName(instance) << ")";
    }

    oss << " - " << GetMessageCount(instance) << " messages";

    if (GetErrorCount(instance) > 0) {
        oss << ", " << GetErrorCount(instance) << " errors";
    }

    return oss.str();
}

std::string LoggerInstanceToolbox::InstanceToJson(const LoggerInstanceData& instance) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"instance_id\":" << instance.instance_id << ",";
    oss << "\"process_id\":" << instance.process_id << ",";
    oss << "\"application_name\":\"" << GetApplicationName(instance) << "\",";
    oss << "\"process_name\":\"" << GetProcessName(instance) << "\",";
    oss << "\"instance_name\":\"" << GetInstanceName(instance) << "\",";
    oss << "\"host_name\":\"" << GetStringField(instance.host_name) << "\",";
    oss << "\"user_name\":\"" << GetStringField(instance.user_name) << "\",";
    oss << "\"creation_time\":" << instance.creation_time << ",";
    oss << "\"last_activity\":" << instance.last_activity << ",";
    oss << "\"message_count\":" << instance.message_count << ",";
    oss << "\"error_count\":" << instance.error_count;
    oss << "}";

    return oss.str();
}

std::string LoggerInstanceToolbox::InstanceToCsv(const LoggerInstanceData& instance) {
    std::ostringstream oss;
    oss << instance.instance_id << ","
        << instance.process_id << ",\""
        << GetApplicationName(instance) << "\",\""
        << GetProcessName(instance) << "\",\""
        << GetInstanceName(instance) << "\",\""
        << GetStringField(instance.host_name) << "\",\""
        << GetStringField(instance.user_name) << "\","
        << instance.creation_time << ","
        << instance.last_activity << ","
        << instance.message_count << ","
        << instance.error_count;

    return oss.str();
}

std::string LoggerInstanceToolbox::FormatInstanceForConsole(const LoggerInstanceData& instance) {
    std::ostringstream oss;
    oss << "Instance " << instance.instance_id << ": "
        << GetApplicationName(instance) << " (PID: " << instance.process_id << ")";

    if (!GetInstanceName(instance).empty()) {
        oss << " [" << GetInstanceName(instance) << "]";
    }

    oss << " - " << GetMessageCount(instance) << " msgs";

    if (GetErrorCount(instance) > 0) {
        oss << ", " << GetErrorCount(instance) << " errs";
    }

    DWORD uptime = CalculateUptime(instance);
    oss << " - uptime: " << uptime << "s";

    return oss.str();
}

// =====================================================================================
// SYSTEM INFORMATION GATHERING
// =====================================================================================

std::pair<DWORD, std::string> LoggerInstanceToolbox::GetCurrentProcessInfo() {
    DWORD process_id = GetCurrentProcessId();
    std::string process_name = "";

    // Try to get process name from executable path
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(NULL, buffer, MAX_PATH) > 0) {
        // Extract just the filename from the full path
        std::string full_path = buffer;
        size_t last_slash = full_path.find_last_of("\\/");
        if (last_slash != std::string::npos) {
            process_name = full_path.substr(last_slash + 1);
        } else {
            process_name = full_path;
        }
    }

    return {process_id, process_name};
}

std::string LoggerInstanceToolbox::GetCurrentUserName() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;

    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }

    return "Unknown";
}

std::string LoggerInstanceToolbox::GetHostName() {
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD hostname_len = MAX_COMPUTERNAME_LENGTH + 1;

    if (GetComputerNameA(hostname, &hostname_len)) {
        return std::string(hostname);
    }

    return "Unknown";
}

DWORD LoggerInstanceToolbox::GetCurrentTimestamp() {
    return static_cast<DWORD>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

std::string LoggerInstanceToolbox::GenerateInstanceName(const std::string& application_name,
                                                       const std::string& process_name) {
    std::ostringstream oss;
    oss << application_name;

    if (!process_name.empty()) {
        oss << "_" << process_name;
    }

    oss << "_" << GetCurrentTimestamp();

    return oss.str();
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

bool LoggerInstanceToolbox::SetStringField(char* field, const std::string& value, size_t max_length) {
    if (value.length() >= max_length) {
        return false; // String too long
    }

    strcpy_s(field, max_length, value.c_str());
    return true;
}

std::string LoggerInstanceToolbox::GetStringField(const char* field) {
    if (!field) return "";
    return std::string(field);
}

uint64_t LoggerInstanceToolbox::CalculateRate(uint64_t count, DWORD uptime_seconds) {
    if (uptime_seconds == 0) return 0;
    return count / uptime_seconds;
}

bool LoggerInstanceToolbox::IsStringEmpty(const char* str) {
    return str == nullptr || str[0] == '\0';
}