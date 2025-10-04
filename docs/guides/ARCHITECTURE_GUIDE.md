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
- **Predictability**: Same input parameters always produce same output

### 🚫 Strict Prohibitions
- ❌ **No logic in POD structs** - Data only, no methods
- ❌ **No state in toolbox classes** - Pure functions only
- ❌ **No direct data manipulation** - Always use toolbox methods
- ❌ **No virtual methods in toolbox** - Direct static calls only
- ❌ **No hard-coded values** - All behavior controlled by parameters
- ❌ **No mutable static state** - Only atomic counters allowed

### 🔧 Stateless Parameterized Pattern

#### Toolbox Layer Requirements
```cpp
// ✅ REQUIRED: Stateless parameterized static methods
class ExampleToolbox {
public:
    // Method signature must include ALL configuration as parameters
    static ReturnType ProcessData(const InputData& input,
                                  const std::string& format_pattern,
                                  bool include_metadata,
                                  size_t max_length) {
        // NO hard-coded values
        // NO instance variables
        // NO mutable static state (except atomic counters)
        // ALL behavior controlled by parameters
        return result;
    }
};
```

#### Stateful Layer Usage Pattern
```cpp
// ✅ CORRECT: Stateful wrapper calling stateless toolbox methods
class ExampleWrapper {
private:
    POD_Data data_;  // Stateless data only

public:
    std::string process(const std::string& format) {
        // Pass ALL configuration as parameters to toolbox method
        return ExampleToolbox::ProcessData(data_, format, true, 1024);
    }
};
```

### 🎛️ Three-Level Usage Pattern

#### **Level 1: Instantiation/Configuration** 🏗️
**Where**: Application startup, configuration phase
**Purpose**: Create and configure logging infrastructure
**Components**: Managers, configuration loaders, initialization code
```cpp
// ✅ CORRECT: Configuration level
auto& instance_manager = GetGlobalInstanceManager();
auto& config_manager = GetGlobalConfigurationManager("MyApp");
auto& monitoring_manager = GetGlobalMonitoringManager("MyApp");

// Configure for application lifecycle
instance_manager.registerInstance("TradingSystem", "OrderProcessor");
config_manager.loadFromFile("config/trading_system.json");
monitoring_manager.startMonitoring();
```

#### **Level 2: Consumer Setup** ⚙️
**Where**: Individual classes/modules that need logging
**Purpose**: Get configured logger instance for consumption
**Components**: Logger instances, consumer-specific setup
```cpp
// ✅ CORRECT: Consumer setup level
class TradingEngine {
private:
    LoggerInstance instance_;
    ImportanceMapper* importance_mapper_;
    ContextualPersistenceManager* persistence_manager_;

public:
    TradingEngine() {
        // Get pre-configured components
        auto& instance_manager = GetGlobalInstanceManager();
        instance_ = instance_manager.findInstance("TradingSystem");

        auto& config_manager = GetGlobalConfigurationManager("TradingSystem");
        importance_mapper_ = new ImportanceMapper("TradingSystem");
        persistence_manager_ = new ContextualPersistenceManager("TradingSystem");
    }
};
```

#### **Level 3: Consumption** 📝
**Where**: Actual logging operations during runtime
**Purpose**: Log messages with minimal overhead using stateless parameterized methods
**Components**: Simple logger method calls with all configuration as parameters
```cpp
// ✅ CORRECT: Consumption level with parameterized ToolBox delegation
void TradingEngine::ProcessOrder(const Order& order) {
    // Simple, fast logging calls using parameterized ToolBox methods
    logger_->info("OrderProcessor", "ProcessOrder",
                  "Processing order {} for customer {}",
                  order.id, order.customer_id);

    // All configuration passed as parameters to ToolBox methods
    if (order.type == OrderType::CRITICAL) {
        logger_->critical("OrderProcessor", "ProcessOrder",
                         "Critical order detected: {}", order.id);
    }
}
```

### 🚫 Incorrect Usage Patterns
```cpp
// ❌ WRONG: Configuration in consumption code
void ProcessOrder(const Order& order) {
    auto logger = Logger::getInstance("TradingSystem");  // Configuration in consumption!
    auto config = LoadConfiguration("config.json");       // Configuration in consumption!
    logger->configure(true, "app.log");                  // Configuration in consumption!
    logger->info("Processing order");                    // Mixed levels!
}

// ❌ WRONG: Heavy operations in logging calls
void ProcessOrder(const Order& order) {
    auto& manager = GetGlobalInstanceManager();          // Heavy operation in consumption!
    auto instance = manager.registerInstance("Temp");    // Heavy operation in consumption!
    logger->info("Processing order");                    // Should be simple call!
}
```

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

### Layer 2: Toolbox Classes (Stateless Parameterized Static Methods)

#### Purpose
- **Pure logic without state**: No instance variables, only parameters control behavior
- **Static methods only**: No object instantiation, direct function calls
- **Parameterized behavior**: All configuration passed as parameters, no hard-coded values
- **Thread-safe by design**: Stateless methods are inherently thread-safe
- **Testable functions**: Pure functions with predictable input/output relationships

#### Core Principles
1. **Stateless Operations**: Methods must not maintain internal state between calls
2. **Parameter-Driven**: All behavior controlled by input parameters
3. **Pure Functions**: Same input parameters always produce same output
4. **No Side Effects**: Methods don't modify external state (except atomic counters)
5. **Immutable Parameters**: Input parameters treated as read-only

#### Coding Standards
```cpp
// ✅ CORRECT: Stateless parameterized toolbox class
class LogMessageToolbox {
private:
    // Only atomic counters for shared state (thread-safe)
    static std::atomic<uint32_t> message_counter_;

public:
    // Pure function: same input = same output, no state changes
    static uint32_t GenerateMessageId() {
        return message_counter_.fetch_add(1);
    }

    // Pure function: validation based only on input parameters
    static bool ValidateMessage(const LogMessageData& data) {
        return data.message_id != 0 && data.process_id != 0;
    }

    // Parameterized function: format pattern passed as parameter
    static std::string MessageToString(const LogMessageData& data,
                                        const std::string& format_pattern) {
        // All formatting controlled by format_pattern parameter
        std::ostringstream oss;
        // Format according to provided pattern
        oss << data.message;
        return oss.str();
    }

    // Factory function: all behavior controlled by input parameters
    static LogMessageData CreateMessage(
        LogMessageType type,                    // Required parameter
        const std::string& content,             // Required parameter
        const std::string& component,           // Optional parameter with default
        const std::string& function,            // Optional parameter with default
        const std::string& file,                // Optional parameter with default
        uint32_t line,                         // Optional parameter with default
        uint32_t instance_id) {                // Required parameter

        LogMessageData data;
        data.message_id = GenerateMessageId();  // Pure function call
        data.type = type;                      // Parameter-driven
        data.instance_id = instance_id;        // Parameter-driven
        // All other fields populated based on input parameters only
        return data;
    }

    // ❌ STRICTLY FORBIDDEN:
    // LogMessageToolbox toolbox;                    // No instances!
    // int instance_var_;                            // No instance variables!
    // virtual void process();                       // No virtual methods!
    // static std::string format = "[%H:%M:%S]";     // No hard-coded values!
    // static const int MAX_SIZE = 1024;             // No hard-coded constants!
    // std::string last_error_;                      // No mutable state!
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

## Codebase Organization Principles

### 📁 Implementation Structure
```
ASFMLogger/
├── src/                          # Core implementation
│   ├── structs/                  # Layer 1: Data structures only
│   ├── toolbox/                  # Layer 2: Pure logic only
│   ├── stateful/                 # Layer 3: State management
│   ├── managers/                 # Coordination layer
│   ├── enhanced/                 # Enhanced implementations
│   ├── utils/                    # Utility classes
│   └── web/                      # Web interface components
├── wrappers/                     # Multi-language bindings
├── docs/                         # Documentation
└── [config files]               # Configuration templates
```

### 🔗 Dependency Flow
```
Configuration Files → Managers → Stateful Objects → Toolbox → POD Structs
                                ↓
                          Enhanced Logger → External APIs
```

### 📊 Layer Responsibilities

#### **POD Structs Layer** (Pure Data)
```cpp
// ✅ Only data, no behavior
struct LogMessageData {
    uint32_t id;           // Data only
    char message[1024];    // Data only
    // No methods!
};
```

#### **Toolbox Layer** (Pure Logic)
```cpp
// ✅ Only logic, no state, no hard-coded values
class LogMessageToolbox {
public:
    static uint32_t GenerateId();  // Pure logic
    static bool Validate(const LogMessageData& data);  // Pure logic
    static std::string Format(const LogMessageData& data,
                              const std::string& format);  // Parameter-driven
};
```

#### **Stateful Layer** (State + Toolbox Usage)
```cpp
// ✅ State management using toolbox
class LogMessage {
private:
    LogMessageData data_;  // State
public:
    std::string format() const {
        return LogMessageToolbox::Format(data_, "[%H:%M:%S] %v");  // Uses toolbox
    }
};
```

#### **Manager Layer** (Coordination)
```cpp
// ✅ Coordinates multiple components
class LoggerInstanceManager {
public:
    LoggerInstance registerInstance(const std::string& app_name) {
        // Coordinates instance creation and registration
        return LoggerInstance(LoggerInstanceToolbox::Create(app_name));
    }
};
```

## File Structure

```
ASFMLogger/
├── 📁 docs/                                    # Documentation and guides
│   ├── ARCHITECTURE_GUIDE.md                   # Complete architecture documentation
│   ├── DEVELOPMENT_PLAN.md                     # Comprehensive development roadmap
│   └── FILE_STRUCTURE.md                       # This file - complete file structure
│
├── 📁 src/                                     # Source code directory
│   ├── 📁 structs/                             # Layer 1: POD data structures
│   │   ├── LogDataStructures.hpp               # Core enums and data structures
│   │   ├── LoggerInstanceData.hpp              # Instance management data
│   │   ├── ImportanceConfiguration.hpp        # Importance framework data
│   │   ├── PersistencePolicy.hpp              # Persistence policy data
│   │   ├── SmartQueueConfiguration.hpp        # Queue configuration data
│   │   ├── DatabaseConfiguration.hpp           # Database configuration data
│   │   ├── ConfigurationData.hpp              # Configuration system data
│   │   ├── MonitoringData.hpp                 # Monitoring and metrics data
│   │   └── CrossLanguageSerializationData.hpp # Cross-language serialization data
│   │
│   ├── 📁 toolbox/                             # Layer 2: Static toolbox classes
│   │   ├── LogMessageToolbox.hpp               # Message manipulation utilities
│   │   ├── TimestampToolbox.hpp                # Timestamp operations
│   │   ├── LoggerInstanceToolbox.hpp           # Instance management logic
│   │   ├── ImportanceToolbox.hpp               # Importance resolution logic
│   │   ├── ContextualPersistenceToolbox.hpp   # Persistence decision logic
│   │   ├── SmartQueueToolbox.hpp               # Queue management logic
│   │   ├── DatabaseToolbox.hpp                 # Database operations logic
│   │   ├── ConfigurationToolbox.hpp            # Configuration parsing logic
│   │   ├── MonitoringToolbox.hpp               # Monitoring and adaptation logic
│   │   └── CrossLanguageSerializationToolbox.hpp # Cross-language serialization
│   │
│   ├── 📁 stateful/                            # Layer 3: Stateful wrapper classes
│   │   ├── LogMessage.hpp                      # Stateful message wrapper
│   │   ├── LoggerInstance.hpp                  # Stateful instance wrapper
│   │   ├── ImportanceMapper.hpp                # Stateful importance configuration
│   │   ├── SmartMessageQueue.hpp               # Stateful queue wrapper
│   │   ├── DatabaseLogger.hpp                  # Stateful database wrapper
│   │   └── CrossLanguageMessage.hpp            # Cross-language message wrapper
│   │
│   ├── 📁 managers/                            # Manager and coordinator classes
│   │   ├── LoggerInstanceManager.hpp           # Instance lifecycle management
│   │   ├── ContextualPersistenceManager.hpp   # Persistence policy management
│   │   ├── MessageQueueManager.hpp             # Queue coordination
│   │   ├── ConfigurationManager.hpp            # Configuration lifecycle
│   │   ├── MonitoringManager.hpp               # Monitoring and adaptation
│   │   └── CrossLanguageBridge.hpp             # Cross-language bridge management
│   │
│   ├── 📁 enhanced/                            # Enhanced implementations
│   │   └── ASFMLoggerEnhanced.cpp              # Enhanced logger implementation
│   │
│   ├── 📁 utils/                               # Utility and helper classes
│   │   └── CrossLanguageSerialization.hpp      # Cross-language serialization utilities
│   │
│   └── 📁 web/                                 # Web interface components
│       ├── RestApiServer.hpp                   # REST API server implementation
│       └── 📁 dashboard/                       # Web dashboard interface
│           ├── index.html                      # Main dashboard HTML
│           ├── styles.css                      # Dashboard CSS styles
│           └── dashboard.js                    # Dashboard JavaScript
│
├── 📁 wrappers/                                # Multi-language wrappers
│   ├── 📁 python/                             # Python language bindings
│   │   └── asfm_logger.py                     # Python ASFMLogger wrapper
│   │
│   ├── 📁 csharp/                             # C# language bindings
│   │   └── ASFMLogger.cs                      # C# ASFMLogger wrapper
│   │
│   └── 📁 mql5/                               # MQL5 language bindings
│       └── ASFMLogger.mq5                     # MQL5 MetaTrader wrapper
│
├── 📄 ASFMLogger.hpp                           # Enhanced main header (backward compatible)
├── 📄 ASFMLogger.cpp                           # Enhanced main implementation
└── 📄 README.md                                # Updated project documentation
```

This architecture ensures ASFMLogger is maintainable, testable, and performant while providing clear separation of concerns and maximum reusability.