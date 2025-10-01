# ASFMLogger Toolbox Architecture Guide

## Overview

ASFMLogger follows a **3-layer architecture** that separates concerns for maximum testability, reusability, and maintainability:

1. **POD Layer**: Pure data structures (Plain Old Data)
2. **Toolbox Layer**: Static stateless methods with pure logic
3. **Stateful Layer**: Objects that internally use toolbox methods

## Architecture Principles

### 🎯 Core Philosophy
- **Testability First**: Pure functions are easily unit testable
- **Reusability**: Toolbox methods work in any context
- **Performance**: No virtual function overhead
- **Thread Safety**: Stateless methods are inherently thread-safe
- **Maintainability**: Clear separation of data, logic, and state

### 🚫 Strict Prohibitions
- ❌ **No logic in POD structs** - Data only, no methods
- ❌ **No state in toolbox classes** - Pure functions only
- ❌ **No direct data manipulation** - Always use toolbox methods
- ❌ **No virtual methods in toolbox** - Direct static calls only

## Layer Definitions

### Layer 1: POD (Plain Old Data) Structures

#### Purpose
- Store data only (no logic)
- Compatible with shared memory and serialization
- Fixed-size for predictable memory layout
- No methods, no virtual functions

#### Coding Standards
```cpp
// ✅ CORRECT: Pure data structure
struct LogMessageData {
    uint32_t message_id;
    uint32_t instance_id;
    LogMessageType type;
    LogTimestamp timestamp;
    DWORD process_id;
    DWORD thread_id;
    char message[1024];  // Fixed size for shared memory
    // No methods allowed!
};

// ✅ CORRECT: Simple data container
struct LogTimestamp {
    DWORD seconds;
    DWORD microseconds;
    DWORD milliseconds;
    // No methods allowed!
};

// ❌ INCORRECT: Methods in POD struct
struct BadLogTimestamp {
    DWORD seconds;
    DWORD toUnixTime() { return seconds; }  // VIOLATION!
};
```

#### File Organization
- `structs/[Component]Data.hpp` - Pure data structures
- No implementation files needed for POD structs

### Layer 2: Toolbox Classes (Static Methods)

#### Purpose
- Pure logic without state
- Static methods only
- No instance variables
- Thread-safe by design

#### Coding Standards
```cpp
// ✅ CORRECT: Pure toolbox class
class LogMessageToolbox {
private:
    static std::atomic<uint32_t> message_counter_;  // Only static variables allowed

public:
    // Pure functions - same input = same output
    static uint32_t GenerateMessageId() {
        return message_counter_.fetch_add(1);
    }

    static bool ValidateMessage(const LogMessageData& data) {  // const reference only
        return data.message_id != 0 && data.process_id != 0;
    }

    static std::string MessageToString(const LogMessageData& data) {
        std::ostringstream oss;
        oss << "Message: " << data.message;
        return oss.str();  // Return by value OK for toolbox
    }

    // Factory methods for creating data structures
    static LogMessageData CreateMessage(LogMessageType type,
                                       const std::string& content) {
        LogMessageData data;
        data.message_id = GenerateMessageId();
        data.type = type;
        // ... populate other fields
        return data;
    }

    // ❌ NEVER DO THIS:
    // LogMessageToolbox toolbox;  // No instances!
    // int instance_var_;           // No instance variables!
    // virtual void process();      // No virtual methods!
};

// ✅ CORRECT: Specialized toolbox for specific operations
class TimestampToolbox {
public:
    static LogTimestamp Now() {
        LogTimestamp ts;
        // ... populate timestamp
        return ts;
    }

    static std::string Format(const LogTimestamp& ts) {
        return FormatTimestamp(ts, true);
    }

private:
    static std::string FormatTimestamp(const LogTimestamp& ts, bool include_microseconds);
};
```

#### File Organization
- `toolbox/[Component]Toolbox.hpp` - Toolbox class declarations
- `toolbox/[Component]Toolbox.cpp` - Toolbox implementations
- `toolbox/[Operation]Toolbox.hpp` - Specialized operation toolboxes

### Layer 3: Stateful Objects

#### Purpose
- Manage state and lifecycle
- Internally use toolbox methods
- Provide object-oriented interface
- Handle resource management

#### Coding Standards
```cpp
// ✅ CORRECT: Stateful object using toolbox internally
class LogMessage {
private:
    LogMessageData data_;  // POD data only

public:
    LogMessage(LogMessageType type, const std::string& content)
        : data_(LogMessageToolbox::CreateMessage(type, content)) {}

    // Public interface uses toolbox internally
    std::string getMessage() const {
        return LogMessageToolbox::ExtractMessage(data_);
    }

    bool isValid() const {
        return LogMessageToolbox::ValidateMessage(data_);
    }

    std::string toString() const {
        return LogMessageToolbox::MessageToString(data_);
    }

    // Setters use toolbox for safe operations
    void setComponent(const std::string& component) {
        LogMessageToolbox::SetComponent(data_, component);
    }

    // ❌ NEVER DO THIS:
    // static uint32_t GenerateId();  // Logic should be in toolbox
    // uint32_t Validate();          // Should use toolbox
};

// ✅ CORRECT: Manager classes for collections
class LogMessageManager {
private:
    std::vector<LogMessage> messages_;
    mutable std::mutex mutex_;

public:
    void addMessage(const LogMessage& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        messages_.push_back(message);
    }

    std::vector<LogMessage> getMessagesByType(LogMessageType type) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<LogMessage> result;

        for (const auto& message : messages_) {
            if (LogMessageToolbox::GetType(message.getData()) == type) {
                result.push_back(message);
            }
        }

        return result;
    }
};
```

#### File Organization
- `stateful/[Component].hpp` - Stateful class declarations
- `stateful/[Component].cpp` - Stateful class implementations
- `managers/[Component]Manager.hpp` - Collection and lifecycle management

## Implementation Patterns

### Factory Pattern with Toolbox
```cpp
// Creating objects using toolbox
LogMessage error_msg = LogMessage::Create(
    LogMessageToolbox::CreateMessage(LogMessageType::ERROR, "Error occurred")
);

// Or using toolbox directly
LogMessageData data = LogMessageToolbox::CreateMessage(LogMessageType::INFO, "Info message");
LogMessage info_msg(data);
```

### Builder Pattern for Complex Objects
```cpp
class LogMessageBuilder {
private:
    LogMessageData data_;

public:
    LogMessageBuilder& setType(LogMessageType type) {
        LogMessageToolbox::SetType(data_, type);
        return *this;
    }

    LogMessageBuilder& setMessage(const std::string& message) {
        LogMessageToolbox::SetMessage(data_, message);
        return *this;
    }

    LogMessageBuilder& setComponent(const std::string& component) {
        LogMessageToolbox::SetComponent(data_, component);
        return *this;
    }

    LogMessage build() const {
        return LogMessage(data_);
    }
};

// Usage
LogMessage msg = LogMessageBuilder()
    .setType(LogMessageType::ERROR)
    .setMessage("Database connection failed")
    .setComponent("Database")
    .build();
```

### Strategy Pattern for Algorithms
```cpp
// Different persistence strategies using toolbox
class PersistenceStrategy {
public:
    virtual bool persist(const LogMessageData& data) = 0;
    virtual ~PersistenceStrategy() = default;
};

class DatabasePersistence : public PersistenceStrategy {
public:
    bool persist(const LogMessageData& data) override {
        return DatabaseToolbox::InsertMessage(data);
    }
};

class FilePersistence : public PersistenceStrategy {
public:
    bool persist(const LogMessageData& data) override {
        return FileToolbox::WriteMessage(data);
    }
};
```

## Testing Strategy

### Toolbox Testing (Unit Tests)
```cpp
TEST(LogMessageToolboxTest, GenerateMessageId) {
    uint32_t id1 = LogMessageToolbox::GenerateMessageId();
    uint32_t id2 = LogMessageToolbox::GenerateMessageId();
    EXPECT_NE(id1, id2);
    EXPECT_LT(id1, id2);  // Should be sequential
}

TEST(LogMessageToolboxTest, ValidateMessage) {
    LogMessageData valid_data = LogMessageToolbox::CreateMessage(LogMessageType::INFO, "test");
    EXPECT_TRUE(LogMessageToolbox::ValidateMessage(valid_data));

    LogMessageData invalid_data;
    memset(&invalid_data, 0, sizeof(LogMessageData));
    EXPECT_FALSE(LogMessageToolbox::ValidateMessage(invalid_data));
}

TEST(TimestampToolboxTest, NowReturnsIncreasingTime) {
    auto ts1 = TimestampToolbox::Now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto ts2 = TimestampToolbox::Now();
    EXPECT_LT(ts1, ts2);
}
```

### Stateful Object Testing (Integration Tests)
```cpp
TEST(LogMessageTest, ConstructorAndGetters) {
    LogMessage msg(LogMessageType::ERROR, "test error");

    EXPECT_TRUE(msg.isValid());
    EXPECT_EQ(msg.getType(), LogMessageType::ERROR);
    EXPECT_EQ(msg.getMessage(), "test error");
}

TEST(LogMessageManagerTest, AddAndRetrieveMessages) {
    LogMessageManager manager;

    LogMessage error_msg(LogMessageType::ERROR, "error");
    LogMessage info_msg(LogMessageType::INFO, "info");

    manager.addMessage(error_msg);
    manager.addMessage(info_msg);

    auto error_messages = manager.getMessagesByType(LogMessageType::ERROR);
    EXPECT_EQ(error_messages.size(), 1);
    EXPECT_EQ(error_messages[0].getMessage(), "error");
}
```

## Performance Considerations

### Toolbox Performance Benefits
- **No virtual calls**: Direct static method invocation
- **Better inlining**: Static methods can be inlined more aggressively
- **No this pointer**: Less register pressure
- **Cache friendly**: No object instances to cache

### Memory Layout Optimization
```cpp
// POD structs for optimal memory layout
struct OptimizedData {
    uint32_t id;           // 4 bytes
    LogMessageType type;   // 4 bytes (enum)
    DWORD timestamp;       // 4 bytes
    char message[1024];    // Fixed size for shared memory
    // Total: predictable size, no padding issues
};
```

## Migration Guide

### From Object-Oriented to Toolbox Pattern

#### Before (Object-Oriented)
```cpp
// ❌ OLD WAY: Logic mixed with data
class LogMessage {
    uint32_t id_;
    std::string message_;

public:
    uint32_t generateId() { return ++id_; }  // Logic in object
    bool validate() const;                   // State-dependent
    std::string format() const;              // Formatting logic
};
```

#### After (Toolbox Pattern)
```cpp
// ✅ NEW WAY: Separated concerns
struct LogMessageData {  // Pure data
    uint32_t id;
    char message[1024];
};

class LogMessageToolbox {  // Pure logic
    static uint32_t GenerateId();
    static bool Validate(const LogMessageData& data);
    static std::string Format(const LogMessageData& data);
};

class LogMessage {  // Stateful object
    LogMessageData data_;
public:
    LogMessage(const std::string& message)
        : data_(LogMessageToolbox::Create(message)) {}

    std::string format() const {
        return LogMessageToolbox::Format(data_);  // Uses toolbox
    }
};
```

## Best Practices

### Toolbox Design
1. **Single Responsibility**: Each toolbox class handles one concern
2. **Pure Functions**: Same input always produces same output
3. **No Side Effects**: Toolbox methods don't modify global state (except for counters)
4. **Thread Safe**: Stateless methods are inherently thread-safe
5. **Well Tested**: High test coverage for all toolbox methods

### Stateful Object Design
1. **Composition Over Inheritance**: Use toolbox internally, not inheritance
2. **RAII Compliance**: Proper resource management
3. **Const Correctness**: Use const methods where possible
4. **Exception Safety**: Strong exception guarantee
5. **Performance Conscious**: Minimal overhead over toolbox usage

### General Guidelines
1. **Prefer Toolbox**: Use toolbox methods for all logic operations
2. **Immutable Data**: Treat POD structs as immutable where possible
3. **Factory Methods**: Use toolbox factory methods for object creation
4. **Validation**: Always validate using toolbox before using data
5. **Documentation**: Document the layer each class belongs to

## File Structure

```
src/
├── structs/           # Layer 1: POD data structures
│   ├── LogMessageData.hpp
│   ├── LogTimestamp.hpp
│   └── ...
├── toolbox/          # Layer 2: Static toolbox classes
│   ├── LogMessageToolbox.hpp
│   ├── LogMessageToolbox.cpp
│   ├── TimestampToolbox.hpp
│   ├── TimestampToolbox.cpp
│   └── ...
├── stateful/         # Layer 3: Stateful objects
│   ├── LogMessage.hpp
│   ├── LogMessage.cpp
│   ├── Timestamp.hpp
│   ├── Timestamp.cpp
│   └── ...
└── managers/         # Collection and lifecycle management
    ├── LogMessageManager.hpp
    ├── LogMessageManager.cpp
    └── ...
```

This architecture ensures ASFMLogger is maintainable, testable, and performant while providing clear separation of concerns and maximum reusability.