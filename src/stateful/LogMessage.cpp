/**
 * ASFMLogger LogMessage Implementation
 *
 * Implementation of the stateful LogMessage wrapper class.
 */

#include "stateful/LogMessage.hpp"
#include "toolbox/TimestampToolbox.hpp"
#include "toolbox/LogMessageToolbox.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

// =====================================================================================
// CONSTRUCTORS AND ASSIGNMENT
// =====================================================================================

LogMessage::LogMessage()
    : data_(LogMessageToolbox::CreateMessage(LogMessageType::INFO, "")) {}

LogMessage::LogMessage(LogMessageType type, const std::string& message,
                       const std::string& component, const std::string& function,
                       const std::string& file, uint32_t line)
    : data_(LogMessageToolbox::CreateMessage(type, message, component, function, file, line)) {}

LogMessage::LogMessage(const LogMessageData& data)
    : data_(data) {}

LogMessage::LogMessage(const LogMessage& other)
    : data_(other.data_) {}

LogMessage::LogMessage(LogMessage&& other) noexcept
    : data_(other.data_) {
    // data_ is already moved, other is left in valid but unspecified state
}

LogMessage& LogMessage::operator=(const LogMessage& other) {
    if (this != &other) {
        data_ = other.data_;
    }
    return *this;
}

LogMessage& LogMessage::operator=(LogMessage&& other) noexcept {
    if (this != &other) {
        data_ = other.data_;
        // other.data_ is left in valid but unspecified state
    }
    return *this;
}

// =====================================================================================
// FACTORY METHODS
// =====================================================================================

LogMessage LogMessage::Create(LogMessageType type, const std::string& message,
                              const std::string& component, const std::string& function,
                              const std::string& file, uint32_t line) {
    // Validate inputs using toolbox
    if (message.empty()) {
        throw std::invalid_argument("Message content cannot be empty");
    }

    // Type validation using range check (enum is defined in structs)
    if (static_cast<int>(type) < 0) {
        throw std::invalid_argument("Invalid message type provided");
    }

    return LogMessage(type, message, component, function, file, line);
}

LogMessage LogMessage::clone() const {
    return LogMessage(data_);
}

// =====================================================================================
// COMPARISON OPERATORS IMPLEMENTATION
// =====================================================================================

bool LogMessage::operator<(const LogMessage& other) const {
    return TimestampToolbox::IsBefore(data_.timestamp, other.data_.timestamp);
}

bool LogMessage::operator>(const LogMessage& other) const {
    return TimestampToolbox::IsAfter(data_.timestamp, other.data_.timestamp);
}

bool LogMessage::operator<=(const LogMessage& other) const {
    return !(*this > other);
}

bool LogMessage::operator>=(const LogMessage& other) const {
    return !(*this < other);
}

// =====================================================================================
// UTILITY METHODS IMPLEMENTATION
// =====================================================================================

void LogMessage::swap(LogMessage& other) noexcept {
    std::swap(data_, other.data_);
}

void LogMessage::clear() {
    data_ = LogMessageToolbox::CreateMessage(LogMessageType::INFO, "");
}

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

void swap(LogMessage& a, LogMessage& b) noexcept {
    a.swap(b);
}

std::ostream& operator<<(std::ostream& os, const LogMessage& message) {
    os << message.toString();
    return os;
}
