#ifndef __ASFM_LOGGER_CORE_HPP__
#define __ASFM_LOGGER_CORE_HPP__

/**
 * ASFMLogger Core Data Structures and Types
 *
 * This header defines the foundational data structures for the enhanced ASFMLogger,
 * including message types, importance levels, timestamps, and core data structures
 * for shared memory compatibility and cross-platform support.
 */

#include <string>
#include <chrono>
#include <cstdint>
#include <Windows.h>

// Forward declarations for Windows types
struct _SYSTEMTIME;
struct tm;

// =====================================================================================
// CORE ENUMS
// =====================================================================================

/**
 * @brief Log message types for classification and handling
 */
enum class LogMessageType : int {
    TRACE = 0,      ///< Detailed debugging information
    DEBUG = 1,      ///< Debug information for development
    INFO = 2,       ///< General information about application flow
    WARN = 3,       ///< Warning messages for potentially harmful situations
    ERR = 4,        ///< Error events that might allow application to continue
    CRITICAL = 5    ///< Critical errors that may cause application termination
};

/**
 * @brief Message importance levels for persistence and handling decisions
 */
enum class MessageImportance : int {
    LOW = 0,        ///< Low importance (debug, trace) - may be evicted
    MEDIUM = 1,     ///< Medium importance (info) - persisted based on context
    HIGH = 2,       ///< High importance (warnings) - typically persisted
    CRITICAL = 3    ///< Critical importance (errors) - always persisted
};

/**
 * @brief Convert LogMessageType to string representation
 * @param type The message type to convert
 * @return String representation of the message type
 */
inline std::string LogMessageTypeToString(LogMessageType type) {
    switch (type) {
        case LogMessageType::TRACE: return "TRACE";
        case LogMessageType::DEBUG: return "DEBUG";
        case LogMessageType::INFO: return "INFO";
        case LogMessageType::WARN: return "WARN";
        case LogMessageType::ERR: return "ERROR";
        case LogMessageType::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert MessageImportance to string representation
 * @param importance The importance level to convert
 * @return String representation of the importance level
 */
inline std::string MessageImportanceToString(MessageImportance importance) {
    switch (importance) {
        case MessageImportance::LOW: return "LOW";
        case MessageImportance::MEDIUM: return "MEDIUM";
        case MessageImportance::HIGH: return "HIGH";
        case MessageImportance::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert string to LogMessageType
 * @param str String representation of message type
 * @return Corresponding LogMessageType or TRACE as default
 */
inline LogMessageType StringToLogMessageType(const std::string& str) {
    if (str == "TRACE") return LogMessageType::TRACE;
    if (str == "DEBUG") return LogMessageType::DEBUG;
    if (str == "INFO") return LogMessageType::INFO;
    if (str == "WARN") return LogMessageType::WARN;
    if (str == "ERROR" || str == "ERR") return LogMessageType::ERR;
    if (str == "CRITICAL") return LogMessageType::CRITICAL;
    return LogMessageType::TRACE; // Default fallback
}

/**
 * @brief Convert string to MessageImportance
 * @param str String representation of importance level
 * @return Corresponding MessageImportance or LOW as default
 */
inline MessageImportance StringToMessageImportance(const std::string& str) {
    if (str == "LOW") return MessageImportance::LOW;
    if (str == "MEDIUM") return MessageImportance::MEDIUM;
    if (str == "HIGH") return MessageImportance::HIGH;
    if (str == "CRITICAL") return MessageImportance::CRITICAL;
    return MessageImportance::LOW; // Default fallback
}

// =====================================================================================
// TIMESTAMP STRUCTURES
// =====================================================================================

/**
 * @brief High-precision timestamp structure compatible with Windows
 */
struct LogTimestamp {
    DWORD seconds;      ///< Seconds since Unix epoch
    DWORD microseconds; ///< Microseconds (0-999999)
    DWORD milliseconds; ///< Alternative millisecond representation

    /**
     * @brief Default constructor - initializes to current time
     */
    LogTimestamp() {
        setToCurrentTime();
    }

    /**
     * @brief Constructor with specific values
     * @param sec Seconds since epoch
     * @param usec Microseconds
     * @param msec Milliseconds (alternative to microseconds)
     */
    LogTimestamp(DWORD sec, DWORD usec, DWORD msec = 0)
        : seconds(sec), microseconds(usec), milliseconds(msec) {}

    /**
     * @brief Set timestamp to current system time
     */
    void setToCurrentTime();

    /**
     * @brief Convert to string representation
     * @return Formatted timestamp string
     */
    std::string toString() const;

    /**
     * @brief Convert to Unix timestamp (seconds since epoch)
     * @return Unix timestamp as uint64_t
     */
    uint64_t toUnixTimestamp() const {
        return static_cast<uint64_t>(seconds);
    }

    /**
     * @brief Convert to time_t for standard library compatibility
     * @return time_t representation
     */
    time_t toTimeT() const {
        return static_cast<time_t>(seconds);
    }

    /**
     * @brief Get total microseconds since epoch
     * @return Microseconds as uint64_t
     */
    uint64_t toMicroseconds() const {
        return static_cast<uint64_t>(seconds) * 1000000ULL + microseconds;
    }

    /**
     * @brief Static factory method to create current timestamp
     * @return Current timestamp
     */
    static LogTimestamp now();

    /**
     * @brief Create timestamp from Unix time
     * @param unix_seconds Seconds since Unix epoch
     * @return LogTimestamp representation
     */
    static LogTimestamp fromUnixTime(time_t unix_seconds);

    /**
     * @brief Create timestamp from Windows SYSTEMTIME
     * @param st Windows SYSTEMTIME structure
     * @return LogTimestamp representation
     */
    static LogTimestamp fromSystemTime(const _SYSTEMTIME& st);

    /**
     * @brief Create timestamp from Windows FILETIME
     * @param ft Windows FILETIME structure
     * @return LogTimestamp representation
     */
    static LogTimestamp fromFileTime(const FILETIME& ft);

    /**
     * @brief Convert to Windows SYSTEMTIME
     * @param st Output SYSTEMTIME structure
     */
    void toSystemTime(_SYSTEMTIME& st) const;

    /**
     * @brief Convert to Windows FILETIME
     * @param ft Output FILETIME structure
     */
    void toFileTime(FILETIME& ft) const;

    /**
     * @brief Comparison operators for sorting and filtering
     */
    bool operator<(const LogTimestamp& other) const;
    bool operator<=(const LogTimestamp& other) const;
    bool operator>(const LogTimestamp& other) const;
    bool operator>=(const LogTimestamp& other) const;
    bool operator==(const LogTimestamp& other) const;
    bool operator!=(const LogTimestamp& other) const;
};

// =====================================================================================
// CORE DATA STRUCTURES
// =====================================================================================

/**
 * @brief C-style structure for log message data
 *
 * This structure is designed for:
 * - Shared memory compatibility (Windows FileMaps)
 * - Inter-process communication
 * - Network transmission
 * - Database storage
 *
 * Fixed-size arrays ensure consistent memory layout across platforms.
 */
struct LogMessageData {
    // Header information
    uint32_t message_id;           ///< Unique message identifier
    uint32_t instance_id;          ///< Logger instance ID
    LogMessageType type;           ///< Message type (enum as int)
    LogTimestamp timestamp;        ///< When message was created

    // Process information
    DWORD process_id;              ///< Windows process ID
    DWORD thread_id;               ///< Windows thread ID

    // Message content (fixed size for shared memory)
    char message[1024];            ///< Actual log message
    char component[128];           ///< Component/subsystem name
    char function[128];            ///< Function/method name
    char file[256];                ///< Source file name

    // Metadata
    uint32_t line_number;          ///< Source line number
    char severity_string[16];      ///< String representation of severity
    char reserved[64];             ///< Future extensibility

    /**
     * @brief Default constructor
     */
    LogMessageData() {
        memset(this, 0, sizeof(LogMessageData));
        timestamp.setToCurrentTime();
        process_id = GetCurrentProcessId();
        thread_id = GetCurrentThreadId();
    }

    /**
     * @brief Constructor with message details
     * @param msg_type Message type
     * @param msg Message content
     * @param comp Component name
     * @param func Function name
     * @param src_file Source file
     * @param line Source line number
     */
    LogMessageData(LogMessageType msg_type, const std::string& msg,
                   const std::string& comp, const std::string& func,
                   const std::string& src_file = "", uint32_t line = 0);

    /**
     * @brief Set string fields with length safety
     * @param msg Message content
     * @param comp Component name
     * @param func Function name
     * @param src_file Source file name
     * @param line Source line number
     */
    void setStringFields(const std::string& msg, const std::string& comp,
                        const std::string& func, const std::string& src_file = "",
                        uint32_t line = 0);

    /**
     * @brief Generate a unique message ID
     * @return New unique message ID
     */
    static uint32_t generateMessageId();

    /**
     * @brief Convert to string representation for debugging
     * @return Human-readable string representation
     */
    std::string toString() const;

    /**
     * @brief Get message content as string
     * @return Message string (null-terminated)
     */
    std::string getMessage() const { return std::string(message); }

    /**
     * @brief Get component name as string
     * @return Component string (null-terminated)
     */
    std::string getComponent() const { return std::string(component); }

    /**
     * @brief Get function name as string
     * @return Function string (null-terminated)
     */
    std::string getFunction() const { return std::string(function); }

    /**
     * @brief Get source file as string
     * @return Source file string (null-terminated)
     */
    std::string getFile() const { return std::string(file); }

    /**
     * @brief Get severity as string
     * @return Severity string (null-terminated)
     */
    std::string getSeverityString() const { return std::string(severity_string); }

    /**
     * @brief Validate the message data structure
     * @return true if structure appears valid
     */
    bool isValid() const;

    /**
     * @brief Get total size of the structure in bytes
     * @return Size of LogMessageData in bytes
     */
    static constexpr size_t size() { return sizeof(LogMessageData); }
};

// =====================================================================================
// UTILITY FUNCTIONS
// =====================================================================================

/**
 * @brief Generate a unique message ID across the system
 * @return Unique 32-bit message identifier
 */
uint32_t GenerateUniqueMessageId();

/**
 * @brief Get current process ID (Windows)
 * @return Current process ID
 */
DWORD GetCurrentProcessId();

/**
 * @brief Get current thread ID (Windows)
 * @return Current thread ID
 */
DWORD GetCurrentThreadId();

/**
 * @brief Safe string copy with length checking
 * @param dest Destination buffer
 * @param src Source string
 * @param dest_size Size of destination buffer
 * @return Number of characters copied
 */
size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size);

/**
 * @brief Safe string copy with length checking and padding
 * @param dest Destination buffer
 * @param src Source string
 * @param dest_size Size of destination buffer
 * @param padding_char Character to use for padding
 * @return Number of characters copied
 */
size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size, char padding_char);

/**
 * @brief Convert null-terminated char array to string
 * @param char_array Null-terminated character array
 * @return String representation
 */
std::string CharArrayToString(const char* char_array);

/**
 * @brief Convert char array with length to string
 * @param char_array Character array
 * @param length Length of the array
 * @return String representation
 */
std::string CharArrayToString(const char* char_array, size_t length);

#endif // __ASFM_LOGGER_CORE_HPP__