# Stateful Layer Implementation Guidelines

## Overview

This document provides comprehensive guidelines for implementing the Stateful Layer components of ASFMLogger. All implementations must follow these guidelines to ensure architectural compliance, thread safety, and proper ToolBox integration.

## Architecture Principles

### 1. Stateful Wrapper Pattern (Stateless Parameterized Delegation)
```cpp
// CORRECT: Stateful wrapper calling stateless parameterized ToolBox methods
class StatefulClass {
private:
    POD_Data data_;                    // Stateless POD data only
    mutable std::mutex state_mutex_;   // Thread safety

public:
    // Constructor using ToolBox for data creation (all parameters passed)
    StatefulClass(const std::string& application_name,
                  const std::string& process_name = "",
                  const std::string& instance_name = "")
        : data_(ToolboxClass::CreateInstance(application_name, process_name, instance_name)),
          state_mutex_() {}

    // Thread-safe accessor using parameterized ToolBox method
    std::string getApplicationName() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return ToolboxClass::GetApplicationName(data_);
    }

    // Thread-safe modifier using parameterized ToolBox method
    bool setApplicationInfo(const std::string& application_name,
                           const std::string& process_name) {
        std::lock_guard<std::mutex> lock(state_mutex_);

        // Validate inputs (stateful wrapper responsibility)
        if (application_name.empty()) {
            return false;
        }

        // Delegate to stateless parameterized ToolBox method
        bool success = ToolboxClass::SetApplicationInfo(data_, application_name, process_name);

        // Update activity if successful (using parameterized ToolBox method)
        if (success) {
            ToolboxClass::UpdateActivity(data_);
        }

        return success;
    }
};
```

### 2. Thread Safety Requirements
- **All public methods**: Must be thread-safe
- **Read operations**: Use `const` methods with `lock_guard`
- **Write operations**: Proper locking order to prevent deadlocks
- **Exception safety**: Strong exception guarantee

### 3. ToolBox Integration Pattern (Stateless Parameterized Delegation)
- **Stateless Delegation**: All business logic delegated to parameterized ToolBox methods
- **Parameter Passing**: All configuration and options passed as method parameters
- **No Logic Duplication**: Never reimplement ToolBox functionality
- **Pure Function Calls**: ToolBox methods are pure functions with predictable input/output

#### Required Integration Pattern
```cpp
// ✅ CORRECT: Stateless parameterized ToolBox delegation
bool StatefulClass::performOperation(const std::string& input,
                                     const std::string& format,
                                     bool include_metadata) {
    std::lock_guard<std::mutex> lock(state_mutex_);

    // ALL configuration passed as parameters to ToolBox method
    return ToolboxClass::PerformOperation(data_, input, format, include_metadata);
}

// ❌ INCORRECT: Hard-coded values in ToolBox calls
bool StatefulClass::performOperation(const std::string& input) {
    // Hard-coded format - VIOLATION!
    return ToolboxClass::PerformOperation(data_, input, "[%H:%M:%S]", false);
}
```

## Implementation Standards

### Constructor Guidelines
```cpp
// REQUIRED: Use ToolBox for data initialization
StatefulClass::StatefulClass(const std::string& application_name)
    : data_(), state_mutex_() {

    // Use ToolBox to create/initialize POD data
    data_ = ToolboxClass::CreateInstance(application_name, "", "");

    // Validate using ToolBox
    if (!ToolboxClass::ValidateInstance(data_)) {
        throw std::runtime_error("Invalid instance data");
    }
}
```

### Thread-Safe Accessor Pattern
```cpp
// REQUIRED: Thread-safe read operations
std::string StatefulClass::getApplicationName() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return ToolboxClass::GetApplicationName(data_);
}
```

### Thread-Safe Modifier Pattern
```cpp
// REQUIRED: Thread-safe write operations
bool StatefulClass::setApplicationInfo(const std::string& application_name,
                                      const std::string& process_name) {
    std::lock_guard<std::mutex> lock(state_mutex_);

    // Validate inputs
    if (application_name.empty()) {
        return false;
    }

    // Use ToolBox for actual operation
    bool success = ToolboxClass::SetApplicationInfo(data_, application_name, process_name);

    // Update activity if successful
    if (success) {
        ToolboxClass::UpdateActivity(data_);
    }

    return success;
}
```

### Factory Method Pattern
```cpp
// REQUIRED: Static factory methods
StatefulClass StatefulClass::Create(const std::string& application_name,
                                   const std::string& process_name,
                                   const std::string& instance_name) {
    return StatefulClass(application_name, process_name, instance_name);
}

StatefulClass StatefulClass::clone() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return StatefulClass(data_);
}
```

## Error Handling Standards

### Exception Safety
```cpp
// REQUIRED: Strong exception guarantee
bool StatefulClass::connect() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (is_connected_) {
        return true; // Already connected
    }

    try {
        // Use ToolBox for connection
        bool success = ToolboxClass::Connect(data_);

        if (success) {
            is_connected_ = true;
            updateConnectionStatistics();
        }

        return success;

    } catch (const std::exception& e) {
        // Log error with context
        logError("Connection failed", e.what());

        // Ensure consistent state
        is_connected_ = false;

        return false;
    }
}
```

### Input Validation
```cpp
// REQUIRED: Comprehensive input validation
bool StatefulClass::setConfiguration(const std::string& config_file) {
    // Validate file exists and is readable
    if (config_file.empty()) {
        logError("Configuration", "Config file path cannot be empty");
        return false;
    }

    // Use ToolBox for configuration loading
    return ToolboxClass::LoadConfiguration(data_, config_file);
}
```

## Performance Guidelines

### Memory Management
- **RAII Compliance**: Proper resource acquisition and release
- **No Memory Leaks**: All allocated resources properly freed
- **Efficient Copying**: Avoid unnecessary data duplication

### Thread Safety Optimization
- **Lock Hierarchy**: Consistent locking order across all classes
- **Lock Scope**: Minimize lock duration for performance
- **Read/Write Separation**: Different strategies for read vs write operations

### ToolBox Delegation
- **Zero Logic Duplication**: Never reimplement ToolBox functionality
- **Efficient Calling**: Minimize ToolBox call overhead
- **State Consistency**: Ensure stateful wrapper stays synchronized

## Testing Requirements

### Unit Testing Standards
- **Coverage**: >90% code coverage for all new functionality
- **Thread Safety**: Concurrent access testing
- **Error Conditions**: All error paths tested
- **Edge Cases**: Boundary condition validation

### Integration Testing Standards
- **ToolBox Compatibility**: No regression in ToolBox functionality
- **Cross-Component**: Compatibility with other stateful components
- **Performance**: No significant overhead compared to ToolBox-only

## Code Quality Standards

### Documentation Requirements
- **Method Documentation**: Complete parameter and return value documentation
- **Thread Safety Notes**: Document which methods are thread-safe
- **Performance Characteristics**: Document performance implications
- **Usage Examples**: Practical examples for complex operations

### Code Style Requirements
- **Consistent Naming**: Follow established naming conventions
- **Error Handling**: Uniform error handling patterns
- **Logging**: Comprehensive debugging and operational logging
- **Comments**: Clear comments explaining complex logic

## Integration Patterns

### LoggerInstance Integration
```cpp
// REQUIRED: LoggerInstance as foundation for other components
class OtherComponent {
private:
    std::shared_ptr<LoggerInstance> logger_instance_;

public:
    OtherComponent(const std::shared_ptr<LoggerInstance>& instance)
        : logger_instance_(instance) {}

    void performOperation() {
        // Use logger instance for context
        std::string app_name = logger_instance_->getApplicationName();

        // Perform operation with context
        // ...
    }
};
```

### Cross-Component Dependencies
```cpp
// REQUIRED: Proper dependency management
class SmartMessageQueue {
private:
    std::shared_ptr<LoggerInstance> owner_instance_;
    std::shared_ptr<ImportanceMapper> importance_mapper_;

public:
    SmartMessageQueue(const std::shared_ptr<LoggerInstance>& instance,
                     const std::shared_ptr<ImportanceMapper>& mapper)
        : owner_instance_(instance), importance_mapper_(mapper) {}

    bool enqueue(const LogMessage& message) {
        // Use importance mapper for priority calculation
        auto importance_result = importance_mapper_->resolveMessageImportance(
            message.getData(), {});

        // Use owner instance for context
        return enqueueWithPriority(message.getData(), importance_result.final_importance);
    }
};
```

## Quality Assurance Checklist

### Pre-Implementation
- [ ] **Architecture Review**: Design pattern compliance validated
- [ ] **ToolBox Integration Plan**: Stateless delegation strategy approved
- [ ] **Thread Safety Design**: Mutex usage and locking order validated
- [ ] **Performance Requirements**: Maximum latency and overhead targets set

### During Implementation
- [ ] **Pattern Compliance**: Follow established stateful wrapper patterns
- [ ] **Thread Safety**: All public methods properly synchronized
- [ ] **Error Handling**: Comprehensive error handling and logging
- [ ] **ToolBox Delegation**: Proper stateless operation delegation

### Post-Implementation
- [ ] **Code Review**: Implementation follows all guidelines
- [ ] **Unit Testing**: All methods tested with edge cases
- [ ] **Integration Testing**: Works with other stateful components
- [ ] **Performance Testing**: Meets performance requirements

## Common Pitfalls to Avoid

### ❌ AVOID: Logic Duplication
```cpp
// WRONG: Reimplementing ToolBox logic
bool StatefulClass::customOperation(const std::string& param) {
    // Don't reimplement - delegate to ToolBox
    return myCustomLogic(data_, param); // Wrong!
}

// CORRECT: Delegate to ToolBox
bool StatefulClass::customOperation(const std::string& param) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return ToolboxClass::CustomOperation(data_, param); // Correct!
}
```

### ❌ AVOID: Thread Safety Violations
```cpp
// WRONG: Non-thread-safe public method
std::string StatefulClass::getValue() {
    return ToolboxClass::GetValue(data_); // No mutex protection!
}

// CORRECT: Thread-safe public method
std::string StatefulClass::getValue() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return ToolboxClass::GetValue(data_); // Proper protection
}
```

### ❌ AVOID: Resource Leaks
```cpp
// WRONG: Potential resource leak
void StatefulClass::initialize() {
    if (!is_initialized_) {
        connection_ = new DatabaseConnection(); // Manual allocation
        is_initialized_ = true;
    }
}

// CORRECT: RAII compliance
void StatefulClass::initialize() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (!is_initialized_) {
        // Use ToolBox for resource management
        connection_config_ = ToolboxClass::CreateConnection(data_);
        is_initialized_ = true;
    }
}
```

## Success Metrics

### Functionality Metrics
- **✅ All stateful classes fully implemented and compiling**
- **✅ Complete object-oriented API surface**
- **✅ Seamless ToolBox Layer integration**
- **✅ Thread-safe stateful operations**

### Quality Metrics
- **✅ RAII compliance for all classes**
- **✅ Factory pattern implementation**
- **✅ Comprehensive error handling**
- **✅ Performance optimization**

### Architecture Metrics
- **✅ ToolBox Layer purity maintained**
- **✅ No logic duplication**
- **✅ Consistent API patterns**
- **✅ Proper stateful/stateless separation**

---

## Implementation Checklist

### For Each Stateful Class
- [ ] **Header Analysis**: Complete method inventory and documentation
- [ ] **ToolBox Mapping**: Identify all ToolBox methods to delegate to
- [ ] **Thread Safety Design**: Plan mutex usage and locking strategy
- [ ] **Constructor Implementation**: Proper RAII and initialization
- [ ] **Factory Methods**: Static creation method implementation
- [ ] **Accessor Methods**: Thread-safe read operations
- [ ] **Modifier Methods**: Thread-safe write operations with validation
- [ ] **Error Handling**: Comprehensive error handling and logging
- [ ] **Testing**: Unit tests for all functionality
- [ ] **Integration Testing**: Cross-component compatibility
- [ ] **Performance Validation**: Overhead measurement and optimization
- [ ] **Documentation**: Complete API documentation and examples

### Cross-Component Integration
- [ ] **Dependency Management**: Proper component dependency handling
- [ ] **Shared Resource Management**: Thread-safe shared state handling
- [ ] **Lifecycle Management**: Proper initialization and cleanup order
- [ ] **Error Propagation**: Consistent error handling across components

**📅 Guidelines Created**: October 2025
**🏗️ Architecture**: Stateful Wrapper Pattern with ToolBox Integration
**🎯 Goal**: Complete enterprise-grade stateful object-oriented API