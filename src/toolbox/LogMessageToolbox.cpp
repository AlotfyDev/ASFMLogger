/**
 * ASFMLogger LogMessage Toolbox Implementation
 *
 * Static methods for log message operations following toolbox architecture.
 * Pure functions with no state, maximum testability and reusability.
 */

#include "LogMessageToolbox.hpp"
#include "ASFMLoggerCore.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <chrono>

// Static variables
std::atomic<uint32_t> LogMessageToolbox::message_counter_{1};

// Utility functions are now defined in ASFMLoggerCore.hpp

// SafeStringCopy and CharArrayToString are now defined in ASFMLoggerCore.cpp

// =====================================================================================
// MESSAGE ID GENERATION
// =====================================================================================

uint32_t LogMessageToolbox::GenerateMessageId() {
    return message_counter_.fetch_add(1, std::memory_order_relaxed);
}

void LogMessageToolbox::ResetMessageIdCounter() {
    message_counter_.store(1, std::memory_order_relaxed);
}

// =====================================================================================
// MESSAGE CREATION AND MANIPULATION
// =====================================================================================

LogMessageData LogMessageToolbox::CreateMessage(
    LogMessageType type,
    const std::string& content,
    const std::string& component,
    const std::string& function,
    const std::string& file,
    uint32_t line) {

    LogMessageData data;
    data.message_id = GenerateMessageId();
    data.type = type;
    data.process_id = GetCurrentProcessIdImpl();
    data.thread_id = GetCurrentThreadIdImpl();
    data.line_number = line;

    // Set string fields safely
    SetMessage(data, content);
    SetComponent(data, component);
    SetFunction(data, function);
    SetSourceLocation(data, file, line);

    // Set severity string
    std::string severity = LogMessageTypeToString(type);
    SafeStringCopy(data.severity_string, severity.c_str(), sizeof(data.severity_string));

    return data;
}

bool LogMessageToolbox::SetMessage(LogMessageData& data, const std::string& message) {
    if (message.empty()) return false;
    return SafeStringCopy(data.message, message, sizeof(data.message)) > 0;
}

bool LogMessageToolbox::SetComponent(LogMessageData& data, const std::string& component) {
    if (component.empty()) return false;
    return SafeStringCopy(data.component, component, sizeof(data.component)) > 0;
}

bool LogMessageToolbox::SetFunction(LogMessageData& data, const std::string& function) {
    if (function.empty()) return false;
    return SafeStringCopy(data.function, function, sizeof(data.function)) > 0;
}

bool LogMessageToolbox::SetSourceLocation(LogMessageData& data, const std::string& file, uint32_t line) {
    if (!file.empty()) {
        SafeStringCopy(data.file, file, sizeof(data.file));
    }
    data.line_number = line;
    return true;
}

void LogMessageToolbox::SetType(LogMessageData& data, LogMessageType type) {
    data.type = type;
    std::string severity = LogMessageTypeToString(type);
    SafeStringCopy(data.severity_string, severity, sizeof(data.severity_string));
}

// =====================================================================================
// MESSAGE VALIDATION
// =====================================================================================

bool LogMessageToolbox::ValidateMessage(const LogMessageData& data) {
    return data.message_id != 0 &&
           data.process_id != 0 &&
           data.type >= LogMessageType::TRACE &&
           data.type <= LogMessageType::CRITICAL_LOG &&
           strlen(data.message) > 0;
}

bool LogMessageToolbox::HasContent(const LogMessageData& data) {
    return strlen(data.message) > 0;
}

bool LogMessageToolbox::HasComponent(const LogMessageData& data) {
    return strlen(data.component) > 0;
}

// =====================================================================================
// MESSAGE EXTRACTION
// =====================================================================================

std::string LogMessageToolbox::ExtractMessage(const LogMessageData& data) {
    return CharArrayToString(data.message);
}

std::string LogMessageToolbox::ExtractComponent(const LogMessageData& data) {
    return CharArrayToString(data.component);
}

std::string LogMessageToolbox::ExtractFunction(const LogMessageData& data) {
    return CharArrayToString(data.function);
}

std::string LogMessageToolbox::ExtractFile(const LogMessageData& data) {
    return CharArrayToString(data.file);
}

LogMessageType LogMessageToolbox::GetType(const LogMessageData& data) {
    return data.type;
}

uint32_t LogMessageToolbox::GetId(const LogMessageData& data) {
    return data.message_id;
}

uint32_t LogMessageToolbox::GetInstanceId(const LogMessageData& data) {
    return data.instance_id;
}

// =====================================================================================
// MESSAGE FORMATTING
// =====================================================================================

std::string LogMessageToolbox::MessageToString(const LogMessageData& data) {
    std::ostringstream oss;
    oss << "[" << LogMessageTypeToString(data.type) << "] "
        << "[" << data.component << "] "
        << data.message;

    if (strlen(data.function) > 0) {
        oss << " (function: " << data.function << ")";
    }

    return oss.str();
}

std::string LogMessageToolbox::MessageToJson(const LogMessageData& data) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"message_id\":" << data.message_id << ",";
    oss << "\"instance_id\":" << data.instance_id << ",";
    oss << "\"type\":\"" << LogMessageTypeToString(data.type) << "\",";
    oss << "\"process_id\":" << data.process_id << ",";
    oss << "\"thread_id\":" << data.thread_id << ",";
    oss << "\"component\":\"" << data.component << "\",";
    oss << "\"function\":\"" << data.function << "\",";
    oss << "\"file\":\"" << data.file << "\",";
    oss << "\"line\":" << data.line_number << ",";
    oss << "\"message\":\"" << data.message << "\",";
    oss << "\"severity\":\"" << data.severity_string << "\"";
    oss << "}";

    return oss.str();
}

std::string LogMessageToolbox::MessageToCsv(const LogMessageData& data) {
    std::ostringstream oss;
    oss << data.message_id << ","
        << data.instance_id << ","
        << static_cast<int>(data.type) << ","
        << data.process_id << ","
        << data.thread_id << ",\""
        << data.component << "\",\""
        << data.function << "\",\""
        << data.file << "\","
        << data.line_number << ",\""
        << data.message << "\",\""
        << data.severity_string << "\"";

    return oss.str();
}

std::string LogMessageToolbox::FormatForConsole(const LogMessageData& data) {
    std::ostringstream oss;
    oss << "[" << data.severity_string << "] "
        << "[" << data.component;

    if (strlen(data.function) > 0) {
        oss << "::" << data.function;
    }
    oss << "] " << data.message;

    return oss.str();
}

// =====================================================================================
// MESSAGE FILTERING AND SEARCHING
// =====================================================================================

std::vector<LogMessageData> LogMessageToolbox::FilterByType(
    const std::vector<LogMessageData>& messages,
    LogMessageType type) {

    std::vector<LogMessageData> result;
    std::copy_if(messages.begin(), messages.end(), std::back_inserter(result),
                 [type](const LogMessageData& msg) { return msg.type == type; });

    return result;
}

std::vector<LogMessageData> LogMessageToolbox::FilterByComponent(
    const std::vector<LogMessageData>& messages,
    const std::string& component) {

    std::vector<LogMessageData> result;
    std::string component_lower = component;
    std::transform(component_lower.begin(), component_lower.end(), component_lower.begin(), ::tolower);

    std::copy_if(messages.begin(), messages.end(), std::back_inserter(result),
                 [&](const LogMessageData& msg) {
                     std::string msg_component = ExtractComponent(msg);
                     std::transform(msg_component.begin(), msg_component.end(), msg_component.begin(), ::tolower);
                     return msg_component.find(component_lower) != std::string::npos;
                 });

    return result;
}

std::vector<LogMessageData> LogMessageToolbox::FilterByImportance(
    const std::vector<LogMessageData>& messages,
    MessageImportance min_importance) {

    std::vector<LogMessageData> result;
    std::copy_if(messages.begin(), messages.end(), std::back_inserter(result),
                 [min_importance](const LogMessageData& msg) {
                     return static_cast<MessageImportance>(msg.type) >= min_importance;
                 });

    return result;
}

std::vector<LogMessageData> LogMessageToolbox::SearchByContent(
    const std::vector<LogMessageData>& messages,
    const std::string& search_term) {

    std::vector<LogMessageData> result;
    std::string search_lower = search_term;
    std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);

    std::copy_if(messages.begin(), messages.end(), std::back_inserter(result),
                 [&](const LogMessageData& msg) {
                     std::string message = ExtractMessage(msg);
                     std::transform(message.begin(), message.end(), message.begin(), ::tolower);
                     return message.find(search_lower) != std::string::npos;
                 });

    return result;
}

// =====================================================================================
// MESSAGE ANALYSIS
// =====================================================================================

std::vector<std::pair<LogMessageType, size_t>> LogMessageToolbox::CountByType(
    const std::vector<LogMessageData>& messages) {

    std::unordered_map<LogMessageType, size_t> counts;
    for (const auto& message : messages) {
        counts[message.type]++;
    }

    std::vector<std::pair<LogMessageType, size_t>> result;
    for (const auto& pair : counts) {
        result.emplace_back(pair.first, pair.second);
    }

    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return result;
}

std::vector<std::pair<std::string, size_t>> LogMessageToolbox::CountByComponent(
    const std::vector<LogMessageData>& messages) {

    std::unordered_map<std::string, size_t> counts;
    for (const auto& message : messages) {
        std::string component = ExtractComponent(message);
        if (!component.empty()) {
            counts[component]++;
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

std::vector<std::string> LogMessageToolbox::GetUniqueComponents(
    const std::vector<LogMessageData>& messages) {

    std::unordered_set<std::string> unique_components;
    for (const auto& message : messages) {
        std::string component = ExtractComponent(message);
        if (!component.empty()) {
            unique_components.insert(component);
        }
    }

    std::vector<std::string> result;
    for (const auto& component : unique_components) {
        result.push_back(component);
    }

    std::sort(result.begin(), result.end());
    return result;
}

double LogMessageToolbox::CalculateMessageRate(const std::vector<LogMessageData>& messages) {
    if (messages.empty()) return 0.0;

    // Find oldest and newest messages
    auto oldest = *std::min_element(messages.begin(), messages.end(),
                                   [](const LogMessageData& a, const LogMessageData& b) {
                                       return a.timestamp.seconds < b.timestamp.seconds;
                                   });

    auto newest = *std::max_element(messages.begin(), messages.end(),
                                    [](const LogMessageData& a, const LogMessageData& b) {
                                        return a.timestamp.seconds < b.timestamp.seconds;
                                    });

    if (oldest.timestamp.seconds == newest.timestamp.seconds) {
        return static_cast<double>(messages.size());
    }

    double time_span = static_cast<double>(newest.timestamp.seconds - oldest.timestamp.seconds);
    return static_cast<double>(messages.size()) / time_span;
}

// =====================================================================================
// BATCH OPERATIONS
// =====================================================================================

std::vector<bool> LogMessageToolbox::ValidateMessages(const std::vector<LogMessageData>& messages) {
    std::vector<bool> results;
    results.reserve(messages.size());

    for (const auto& message : messages) {
        results.push_back(ValidateMessage(message));
    }

    return results;
}

void LogMessageToolbox::SortByTimestamp(std::vector<LogMessageData>& messages) {
    std::sort(messages.begin(), messages.end(),
              [](const LogMessageData& a, const LogMessageData& b) {
                  if (a.timestamp.seconds != b.timestamp.seconds) {
                      return a.timestamp.seconds < b.timestamp.seconds;
                  }
                  return a.timestamp.microseconds < b.timestamp.microseconds;
              });
}

void LogMessageToolbox::SortByType(std::vector<LogMessageData>& messages) {
    std::sort(messages.begin(), messages.end(),
              [](const LogMessageData& a, const LogMessageData& b) {
                  return static_cast<int>(a.type) < static_cast<int>(b.type);
              });
}

size_t LogMessageToolbox::RemoveDuplicates(std::vector<LogMessageData>& messages) {
    if (messages.empty()) return 0;

    // Sort by content hash for efficient deduplication
    std::sort(messages.begin(), messages.end(),
              [](const LogMessageData& a, const LogMessageData& b) {
                  return HashMessageContent(a) < HashMessageContent(b);
              });

    auto last = std::unique(messages.begin(), messages.end(),
                           [](const LogMessageData& a, const LogMessageData& b) {
                               return HashMessageContent(a) == HashMessageContent(b) &&
                                      strcmp(a.message, b.message) == 0;
                           });

    size_t removed_count = std::distance(last, messages.end());
    messages.erase(last, messages.end());

    return removed_count;
}

// =====================================================================================
// MESSAGE HASHING AND COMPARISON
// =====================================================================================

uint32_t LogMessageToolbox::HashMessageContent(const LogMessageData& data) {
    return CalculateContentHash(data.message);
}

bool LogMessageToolbox::CompareMessages(const LogMessageData& a, const LogMessageData& b) {
    return a.message_id == b.message_id &&
           a.type == b.type &&
           strcmp(a.message, b.message) == 0 &&
           strcmp(a.component, b.component) == 0 &&
           strcmp(a.function, b.function) == 0;
}

bool LogMessageToolbox::CompareMessageContent(const LogMessageData& a, const LogMessageData& b) {
    return strcmp(a.message, b.message) == 0 &&
           strcmp(a.component, b.component) == 0 &&
           strcmp(a.function, b.function) == 0;
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

uint32_t LogMessageToolbox::CalculateContentHash(const char* content) {
    uint32_t hash = 0;
    while (*content) {
        hash = hash * 31 + static_cast<unsigned char>(*content);
        content++;
    }
    return hash;
}

bool LogMessageToolbox::AreStringsEqual(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

size_t LogMessageToolbox::FindStringLength(const char* str, size_t max_length) {
    size_t length = 0;
    while (length < max_length && str[length] != '\0') {
        length++;
    }
    return length;
}