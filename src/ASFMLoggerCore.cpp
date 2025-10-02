/**
 * ASFMLogger Core Implementation
 *
 * Implementation of core data structures, utilities, and helper functions
 * for the enhanced ASFMLogger system.
 */

#include "ASFMLoggerCore.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <Windows.h>
#include <time.h>

// =====================================================================================
// STATIC VARIABLES
// =====================================================================================

static std::atomic<uint32_t> global_message_id_counter_{1};

// =====================================================================================
// LOGTIMESTAMP IMPLEMENTATION
// =====================================================================================

void LogTimestamp::setToCurrentTime() {
    // Get current system time with high precision
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);

    // Convert FILETIME to Unix timestamp
    ULARGE_INTEGER ull;
    ull.LowPart = file_time.dwLowDateTime;
    ull.HighPart = file_time.dwHighDateTime;

    // FILETIME is in 100-nanosecond intervals since January 1, 1601
    // Unix epoch is January 1, 1970
    const uint64_t FILETIME_UNIX_EPOCH_DIFF = 116444736000000000ULL;

    uint64_t total_100ns_intervals = ull.QuadPart - FILETIME_UNIX_EPOCH_DIFF;
    uint64_t total_seconds = total_100ns_intervals / 10000000ULL;
    uint64_t remaining_100ns = total_100ns_intervals % 10000000ULL;

    seconds = static_cast<DWORD>(total_seconds);
    microseconds = static_cast<DWORD>(remaining_100ns / 10ULL); // Convert to microseconds
    milliseconds = static_cast<DWORD>(remaining_100ns / 10000ULL); // Alternative representation
}

std::string LogTimestamp::toString() const {
    std::ostringstream oss;
    oss << seconds << "." << std::setfill('0') << std::setw(6) << microseconds;
    return oss.str();
}

LogTimestamp LogTimestamp::now() {
    LogTimestamp ts;
    ts.setToCurrentTime();
    return ts;
}

LogTimestamp LogTimestamp::fromUnixTime(time_t unix_seconds) {
    return LogTimestamp(static_cast<DWORD>(unix_seconds), 0, 0);
}

LogTimestamp LogTimestamp::fromSystemTime(const _SYSTEMTIME& st) {
    // Convert SYSTEMTIME to Unix timestamp
    tm tm_time = {};
    tm_time.tm_year = st.wYear - 1900;
    tm_time.tm_mon = st.wMonth - 1;
    tm_time.tm_mday = st.wDay;
    tm_time.tm_hour = st.wHour;
    tm_time.tm_min = st.wMinute;
    tm_time.tm_sec = st.wSecond;
    tm_time.tm_isdst = -1;

    time_t unix_time = mktime(&tm_time);

    // Add milliseconds as microseconds
    DWORD microseconds = st.wMilliseconds * 1000;

    return LogTimestamp(static_cast<DWORD>(unix_time), microseconds, st.wMilliseconds);
}

LogTimestamp LogTimestamp::fromFileTime(const FILETIME& ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    const uint64_t FILETIME_UNIX_EPOCH_DIFF = 116444736000000000ULL;
    uint64_t total_100ns_intervals = ull.QuadPart - FILETIME_UNIX_EPOCH_DIFF;

    uint64_t total_seconds = total_100ns_intervals / 10000000ULL;
    uint64_t remaining_100ns = total_100ns_intervals % 10000000ULL;

    return LogTimestamp(
        static_cast<DWORD>(total_seconds),
        static_cast<DWORD>(remaining_100ns / 10ULL),
        static_cast<DWORD>(remaining_100ns / 10000ULL)
    );
}

void LogTimestamp::toSystemTime(_SYSTEMTIME& st) const {
    // Convert Unix timestamp back to SYSTEMTIME
    time_t time_seconds = static_cast<time_t>(seconds);
    tm tm_time;

    #ifdef _MSC_VER
        localtime_s(&tm_time, &time_seconds);
    #else
        tm_time = *localtime(&time_seconds);
    #endif

    st.wYear = static_cast<WORD>(tm_time.tm_year + 1900);
    st.wMonth = static_cast<WORD>(tm_time.tm_mon + 1);
    st.wDayOfWeek = static_cast<WORD>(tm_time.tm_wday);
    st.wDay = static_cast<WORD>(tm_time.tm_mday);
    st.wHour = static_cast<WORD>(tm_time.tm_hour);
    st.wMinute = static_cast<WORD>(tm_time.tm_min);
    st.wSecond = static_cast<WORD>(tm_time.tm_sec);
    st.wMilliseconds = static_cast<WORD>(milliseconds);
}

void LogTimestamp::toFileTime(FILETIME& ft) const {
    // Convert Unix timestamp to FILETIME
    uint64_t total_seconds = static_cast<uint64_t>(seconds);
    uint64_t total_100ns_intervals = total_seconds * 10000000ULL;

    // Add Unix epoch offset to FILETIME base
    const uint64_t UNIX_EPOCH_FILETIME_OFFSET = 116444736000000000ULL;
    total_100ns_intervals += UNIX_EPOCH_FILETIME_OFFSET;

    ULARGE_INTEGER ull;
    ull.QuadPart = total_100ns_intervals;

    ft.dwLowDateTime = ull.LowPart;
    ft.dwHighDateTime = ull.HighPart;
}

bool LogTimestamp::operator<(const LogTimestamp& other) const {
    if (seconds < other.seconds) return true;
    if (seconds > other.seconds) return false;
    return microseconds < other.microseconds;
}

bool LogTimestamp::operator<=(const LogTimestamp& other) const {
    return !(*this > other);
}

bool LogTimestamp::operator>(const LogTimestamp& other) const {
    if (seconds > other.seconds) return true;
    if (seconds < other.seconds) return false;
    return microseconds > other.microseconds;
}

bool LogTimestamp::operator>=(const LogTimestamp& other) const {
    return !(*this < other);
}

bool LogTimestamp::operator==(const LogTimestamp& other) const {
    return seconds == other.seconds && microseconds == other.microseconds;
}

bool LogTimestamp::operator!=(const LogTimestamp& other) const {
    return !(*this == other);
}

// =====================================================================================
// LOGMESSAGEDATA IMPLEMENTATION
// =====================================================================================

LogMessageData::LogMessageData(LogMessageType msg_type, const std::string& msg,
                               const std::string& comp, const std::string& func,
                               const std::string& src_file, uint32_t line) {
    memset(this, 0, sizeof(LogMessageData));
    timestamp.setToCurrentTime();
    process_id = GetCurrentProcessId();
    thread_id = GetCurrentThreadId();

    type = msg_type;
    message_id = generateMessageId();
    setStringFields(msg, comp, func, src_file, line);

    // Set severity string based on message type
    std::string severity = LogMessageTypeToString(msg_type);
    SafeStringCopy(severity_string, severity, sizeof(severity_string));
}

void LogMessageData::setStringFields(const std::string& msg, const std::string& comp,
                                     const std::string& func, const std::string& src_file,
                                     uint32_t line) {
    SafeStringCopy(message, msg, sizeof(message));
    SafeStringCopy(component, comp, sizeof(component));
    SafeStringCopy(function, func, sizeof(function));
    SafeStringCopy(file, src_file, sizeof(file));
    line_number = line;

    // Update severity string if not already set
    if (strlen(severity_string) == 0) {
        std::string severity = LogMessageTypeToString(type);
        SafeStringCopy(severity_string, severity, sizeof(severity_string));
    }
}

uint32_t LogMessageData::generateMessageId() {
    return global_message_id_counter_.fetch_add(1, std::memory_order_relaxed);
}

std::string LogMessageData::toString() const {
    std::ostringstream oss;
    oss << "LogMessageData{";
    oss << "ID:" << message_id;
    oss << ", Instance:" << instance_id;
    oss << ", Type:" << LogMessageTypeToString(type);
    oss << ", Time:" << timestamp.toString();
    oss << ", PID:" << process_id;
    oss << ", TID:" << thread_id;
    oss << ", Component:'" << component << "'";
    oss << ", Function:'" << function << "'";
    oss << ", Message:'" << message << "'";
    oss << "}";
    return oss.str();
}

bool LogMessageData::isValid() const {
    return message_id != 0 &&
           process_id != 0 &&
           type >= LogMessageType::TRACE &&
           type <= LogMessageType::CRITICAL_LOG &&
           strlen(message) > 0;
}

// =====================================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =====================================================================================

uint32_t GenerateUniqueMessageId() {
    return global_message_id_counter_.fetch_add(1, std::memory_order_relaxed);
}

size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size) {
    return SafeStringCopy(dest, src, dest_size, '\0');
}

size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size, char padding_char) {
    if (!dest || dest_size == 0) {
        return 0;
    }

    size_t copy_length = (src.length() < dest_size - 1) ? src.length() : (dest_size - 1);
    memcpy(dest, src.c_str(), copy_length);
    dest[copy_length] = padding_char;

    // Fill remaining space with padding character
    for (size_t i = copy_length + 1; i < dest_size; ++i) {
        dest[i] = padding_char;
    }

    return copy_length;
}

std::string CharArrayToString(const char* char_array) {
    if (!char_array) {
        return std::string();
    }
    return std::string(char_array);
}

std::string CharArrayToString(const char* char_array, size_t length) {
    if (!char_array || length == 0) {
        return std::string();
    }

    // Find null terminator within the length
    size_t actual_length = 0;
    for (size_t i = 0; i < length; ++i) {
        if (char_array[i] == '\0') {
            actual_length = i;
            break;
        }
        actual_length = length;
    }

    return std::string(char_array, actual_length);
}

// =====================================================================================
// WINDOWS API WRAPPERS
// =====================================================================================

DWORD GetCurrentProcessId() {
    return ::GetCurrentProcessId();
}

DWORD GetCurrentThreadId() {
    return ::GetCurrentThreadId();
}

// =====================================================================================
// ADDITIONAL UTILITY FUNCTIONS
// =====================================================================================

/**
 * @brief Convert time_t to LogTimestamp
 * @param time_seconds Unix timestamp in seconds
 * @return LogTimestamp representation
 */
LogTimestamp TimeTToLogTimestamp(time_t time_seconds) {
    return LogTimestamp::fromUnixTime(time_seconds);
}

/**
 * @brief Convert LogTimestamp to time_t
 * @param timestamp LogTimestamp to convert
 * @return time_t representation
 */
time_t LogTimestampToTimeT(const LogTimestamp& timestamp) {
    return timestamp.toTimeT();
}

/**
 * @brief Get current system time as LogTimestamp
 * @return Current timestamp with high precision
 */
LogTimestamp GetCurrentLogTimestamp() {
    return LogTimestamp::now();
}

/**
 * @brief Format timestamp for display
 * @param timestamp Timestamp to format
 * @param include_microseconds Whether to include microsecond precision
 * @return Formatted timestamp string
 */
std::string FormatTimestamp(const LogTimestamp& timestamp, bool include_microseconds) {
    std::ostringstream oss;

    // Convert to time_t and format as human-readable time
    time_t time_seconds = timestamp.toTimeT();
    tm tm_time;

    #ifdef _MSC_VER
        localtime_s(&tm_time, &time_seconds);
    #else
        tm_time = *localtime(&time_seconds);
    #endif

    oss << std::setfill('0') << std::setw(4) << (tm_time.tm_year + 1900) << "-";
    oss << std::setfill('0') << std::setw(2) << (tm_time.tm_mon + 1) << "-";
    oss << std::setfill('0') << std::setw(2) << tm_time.tm_mday << " ";
    oss << std::setfill('0') << std::setw(2) << tm_time.tm_hour << ":";
    oss << std::setfill('0') << std::setw(2) << tm_time.tm_min << ":";
    oss << std::setfill('0') << std::setw(2) << tm_time.tm_sec;

    if (include_microseconds && timestamp.microseconds > 0) {
        oss << "." << std::setfill('0') << std::setw(6) << timestamp.microseconds;
    }

    return oss.str();
}

/**
 * @brief Create LogMessageData from basic parameters
 * @param type Message type
 * @param message Message content
 * @param component Component name (optional)
 * @param function Function name (optional)
 * @param file Source file (optional)
 * @param line Source line number (optional)
 * @return Configured LogMessageData structure
 */
LogMessageData CreateLogMessageData(
    LogMessageType type,
    const std::string& message,
    const std::string& component,
    const std::string& function,
    const std::string& file,
    uint32_t line) {

    LogMessageData data(type, message, component, function, file, line);
    return data;
}

/**
 * @brief Validate message data structure
 * @param data Message data to validate
 * @return true if structure is valid
 */
bool ValidateLogMessageData(const LogMessageData& data) {
    return data.isValid();
}

/**
 * @brief Calculate hash of message content for deduplication
 * @param data Message data to hash
 * @return 32-bit hash of message content
 */
uint32_t HashLogMessageContent(const LogMessageData& data) {
    uint32_t hash = 0;
    const char* content = data.message;

    while (*content) {
        hash = hash * 31 + static_cast<unsigned char>(*content);
        content++;
    }

    return hash;
}