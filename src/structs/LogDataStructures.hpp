#ifndef __ASFM_LOGGER_DATA_STRUCTURES_HPP__
#define __ASFM_LOGGER_DATA_STRUCTURES_HPP__

/**
 * ASFMLogger POD (Plain Old Data) Structures
 *
 * Pure data structures with no methods or logic.
 * Following toolbox architecture principles.
 */

#include <cstdint>
#include <Windows.h>

// =====================================================================================
// CORE ENUMS (Global constants)
// =====================================================================================

/**
 * @brief Log message types for classification and handling
 */
enum LogMessageType : int {
    TRACE = 0,      ///< Detailed debugging information
    DEBUG = 1,      ///< Debug information for development
    INFO = 2,       ///< General information about application flow
    WARN = 3,       ///< Warning messages for potentially harmful situations
    ERR = 4,        ///< Error events that might allow application to continue
    CRITICAL_LOG = 5    ///< Critical errors that may cause application termination
};

/**
 * @brief Message importance levels for persistence and handling decisions
 */
enum MessageImportance : int {
    LOW = 0,        ///< Low importance (debug, trace) - may be evicted
    MEDIUM = 1,     ///< Medium importance (info) - persisted based on context
    HIGH = 2,       ///< High importance (warnings) - typically persisted
    CRITICAL = 3    ///< Critical importance (errors) - always persisted
};

// =====================================================================================
// TIMESTAMP DATA STRUCTURE
// =====================================================================================

/**
 * @brief High-precision timestamp structure compatible with Windows
 * POD structure - no methods, pure data
 */
struct LogTimestamp {
    DWORD seconds;      ///< Seconds since Unix epoch
    DWORD microseconds; ///< Microseconds (0-999999)
    DWORD milliseconds; ///< Alternative millisecond representation
};

// =====================================================================================
// CORE MESSAGE DATA STRUCTURE
// =====================================================================================

/**
 * @brief C-style structure for log message data
 * POD structure designed for:
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
};

// =====================================================================================
// STRING CONVERSION DATA
// =====================================================================================

/**
 * @brief Data structure for string conversion operations
 * Used by StringConverter toolbox for bidirectional conversions
 */
struct StringConversionData {
    char buffer[4096];             ///< Conversion buffer
    size_t length;                 ///< Actual string length
    size_t capacity;               ///< Buffer capacity
    bool is_wide_char;             ///< Whether this is wide character string
    char padding_char;             ///< Character used for padding
};

#endif // __ASFM_LOGGER_DATA_STRUCTURES_HPP__