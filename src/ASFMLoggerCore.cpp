/**
 * ASFMLogger Core Implementation
 *
 * Implementation of core utility functions for the enhanced ASFMLogger system.
 * Following toolbox architecture - POD structs have no methods, only toolbox classes.
 */

#include "ASFMLoggerCore.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <Windows.h>
#include <time.h>
#include <cstring>

// =====================================================================================
// STATIC VARIABLES
// =====================================================================================

static std::atomic<uint32_t> global_message_id_counter_{1};

// =====================================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =====================================================================================

// Generate a unique message ID across the system
uint32_t GenerateUniqueMessageId() {
    return global_message_id_counter_.fetch_add(1);
}

// Get current process ID (Windows) - renamed to avoid conflicts
DWORD GetCurrentProcessIdImpl() {
    return ::GetCurrentProcessId();
}

// Get current thread ID (Windows) - renamed to avoid conflicts
DWORD GetCurrentThreadIdImpl() {
    return ::GetCurrentThreadId();
}

// Safe string copy with length checking
size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size) {
    if (!dest || dest_size == 0) return 0;

    size_t src_len = src.length();
    size_t copy_size = (src_len < dest_size - 1) ? src_len : (dest_size - 1);
    memcpy(dest, src.c_str(), copy_size);
    dest[copy_size] = '\0';

    return copy_size;
}

// Safe string copy with length checking and padding
size_t SafeStringCopy(char* dest, const std::string& src, size_t dest_size, char padding_char) {
    if (!dest || dest_size == 0) return 0;

    size_t src_len = src.length();
    size_t copy_size = (src_len < dest_size - 1) ? src_len : (dest_size - 1);
    memcpy(dest, src.c_str(), copy_size);

    // Pad remaining space with padding character
    for (size_t i = copy_size; i < dest_size - 1; ++i) {
        dest[i] = padding_char;
    }
    dest[dest_size - 1] = '\0';

    return copy_size;
}

// Convert null-terminated char array to string
std::string CharArrayToString(const char* char_array) {
    if (!char_array) return "";
    return std::string(char_array);
}

// Convert char array with length to string
std::string CharArrayToString(const char* char_array, size_t length) {
    if (!char_array || length == 0) return "";
    return std::string(char_array, length);
}