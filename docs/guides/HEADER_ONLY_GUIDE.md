# ASFMLogger Header-Only Usage Guide

## Overview

ASFMLogger provides a **header-only** approach for maximum ease of integration. This approach eliminates the need to build and link against a library - you simply include the header files and start using ASFMLogger immediately.

## Quick Start

### Basic Header-Only Usage

```cpp
// Define header-only mode BEFORE including ASFMLogger.hpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

// Use ASFMLogger immediately - no building required!
auto logger = Logger::getInstance("MyApp");

// Start logging
logger->info("Application started");
logger->error("Something went wrong: {}", error_message);
```

### With CMake

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MyApp)

# Enable header-only ASFMLogger
add_executable(MyApp main.cpp)
target_compile_definitions(MyApp PRIVATE ASFMLOGGER_HEADER_ONLY)
target_include_directories(MyApp PRIVATE path/to/ASFMLogger)
```

## Header-Only vs Built Library

| Feature | Header-Only | Built Library |
|---------|-------------|---------------|
| **Setup Complexity** | ⭐⭐⭐⭐⭐ (Just include headers) | ⭐⭐⭐ (Build library first) |
| **Build Time** | ⭐⭐⭐⭐⭐ (No compilation needed) | ⭐⭐⭐ (Library compilation) |
| **Features** | ⭐⭐⭐ (Basic logging) | ⭐⭐⭐⭐⭐ (Full enterprise features) |
| **Performance** | ⭐⭐⭐⭐ (Direct calls) | ⭐⭐⭐⭐⭐ (Optimized code) |
| **File Size** | ⭐⭐⭐⭐⭐ (Minimal) | ⭐⭐⭐ (Larger binary) |
| **Integration** | ⭐⭐⭐⭐⭐ (Copy-paste simple) | ⭐⭐⭐ (Link dependencies) |

## Implementation Details

### What You Get

The header-only version provides:
- **Basic logging**: TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL
- **File and console output**: Rotating file sinks with console output
- **Thread safety**: Atomic operations and proper synchronization
- **Error handling**: Last error storage and retrieval
- **Configuration**: Log level and output configuration

### What You Don't Get

The header-only version does **not** include:
- **Multi-instance management**: Advanced instance tracking
- **Importance framework**: Smart message classification
- **Database integration**: SQL Server logging
- **Shared memory**: Inter-process communication
- **Web dashboard**: Real-time monitoring interface
- **Advanced queue management**: Smart message buffering

## Usage Examples

### Simple Application

```cpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

int main() {
    // Get logger instance
    auto logger = Logger::getInstance("MySimpleApp");

    // Configure logging
    logger->configure(
        true,                           // console_output
        "application.log",              // log_file_name
        10485760,                       // max_file_size (10MB)
        5,                              // max_files
        spdlog::level::info             // log_level
    );

    // Start logging
    logger->info("Application starting up");
    logger->debug("Debug information: {}", some_variable);
    logger->warn("This is a warning message");
    logger->error("An error occurred: {}", error_details);
    logger->critical("Critical system failure!");

    return 0;
}
```

### CMake Integration

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MyApplication)

# Create executable
add_executable(MyApp main.cpp)

# Enable header-only ASFMLogger
target_compile_definitions(MyApp PRIVATE ASFMLOGGER_HEADER_ONLY)

# Include ASFMLogger headers
target_include_directories(MyApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../ASFMLogger
)

# No linking required for header-only usage!
```

### Advanced Configuration

```cpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

void configureLogging() {
    auto logger = Logger::getInstance("AdvancedApp");

    // Configure with custom settings
    logger->configure(
        true,                           // Enable console output
        "logs/advanced_app.log",        // Custom log file path
        52428800,                       // 50MB max file size
        10,                             // Keep 10 files
        spdlog::level::debug            // Debug level logging
    );

    // Verify configuration
    if (logger->isConfigured()) {
        logger->info("Logging configured successfully");
    }
}
```

## File Structure

For header-only usage, you only need:

```
ASFMLogger/
├── ASFMLogger.hpp              # Main header with mode detection
├── ASFMLogger_HeaderOnly.hpp   # Header-only implementation
└── src/
    └── structs/
        └── LogDataStructures.hpp   # Core data structures
```

## Platform Support

### Windows

```cpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

// Works immediately on Windows
auto logger = Logger::getInstance("WindowsApp");
logger->info("Running on Windows");
```

### Linux/macOS

```cpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

// Cross-platform compatibility
auto logger = Logger::getInstance("CrossPlatformApp");
logger->info("Running on {}", getPlatformName());
```

## Performance Considerations

### Header-Only Performance

**Advantages:**
- **No linking overhead**: Direct function calls
- **Better inlining**: Compiler can optimize across boundaries
- **Smaller binaries**: No library dependencies
- **Faster compilation**: No library build step

**Considerations:**
- **Larger executable**: Implementation code included in every binary
- **Slower compilation**: More code to compile for each project
- **Limited features**: Only basic logging functionality

### Built Library Performance

**Advantages:**
- **Smaller executables**: Shared library implementation
- **Full features**: All enterprise features available
- **Better optimization**: Library can be highly optimized
- **Faster project compilation**: Only headers parsed

## Migration Guide

### From Built Library to Header-Only

```cpp
// Before (Built Library)
#include "ASFMLogger.hpp"
auto logger = Logger::getInstance("MyApp");
logger->info("Message");

// After (Header-Only)
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"
auto logger = Logger::getInstance("MyApp");
logger->info("Message");  // Same API!
```

### From Header-Only to Built Library

```cpp
// Before (Header-Only)
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

// After (Built Library)
// Remove the define, link against ASFMLogger library
#include "ASFMLogger.hpp"
target_link_libraries(MyApp ASFMLogger)
```

## Troubleshooting

### Common Issues

#### Compilation Errors
```cpp
// Make sure to define header-only mode BEFORE including the header
#define ASFMLOGGER_HEADER_ONLY  // This must come first
#include "ASFMLogger.hpp"
```

#### Multiple Definitions
```cpp
// If you get multiple definition errors, ensure header-only mode
// is defined in ALL source files that include ASFMLogger.hpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"
```

#### Missing Features
```cpp
// Header-only version has limited features
// For full functionality, use the built library approach
#define ASFMLOGGER_HEADER_ONLY  // Limited features
// Remove define for full features (requires building library)
```

### Performance Issues

#### Large Executable Size
```cpp
// Header-only includes implementation in every binary
// For smaller size, use built library approach
// Remove ASFMLOGGER_HEADER_ONLY define and build ASFMLogger library
```

#### Slow Compilation
```cpp
// Header-only compiles more code per project
// For faster builds, use pre-built ASFMLogger library
// Remove ASFMLOGGER_HEADER_ONLY define
```

## Best Practices

### When to Use Header-Only

✅ **Small projects**: Quick integration without build complexity
✅ **Prototyping**: Fast development without library setup
✅ **Simple applications**: Basic logging needs
✅ **CI/CD environments**: No complex build dependencies
✅ **Embedded systems**: Minimal external dependencies

### When to Use Built Library

✅ **Large projects**: Better performance and smaller executables
✅ **Enterprise applications**: Full feature set required
✅ **Multiple applications**: Shared library reduces memory usage
✅ **High-performance requirements**: Optimized compiled code
✅ **Advanced features**: Database, shared memory, web dashboard

### Header-Only Best Practices

1. **Define mode early**: Set ASFMLOGGER_HEADER_ONLY before any includes
2. **Consistent usage**: Use header-only mode across entire project
3. **Minimal dependencies**: Perfect for projects with few external deps
4. **Simple integration**: Ideal for quick proof-of-concepts

## Integration Examples

### Simple CMake Project

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(SimpleApp)

add_executable(SimpleApp main.cpp)
target_compile_definitions(SimpleApp PRIVATE ASFMLOGGER_HEADER_ONLY)
target_include_directories(SimpleApp PRIVATE ../ASFMLogger)
```

```cpp
// main.cpp
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

int main() {
    auto logger = Logger::getInstance("SimpleApp");
    logger->configure(true, "simple.log", 10485760, 5, spdlog::level::info);

    logger->info("Simple application started");
    logger->error("Test error message");

    return 0;
}
```

### Visual Studio Project

```cpp
// Project Properties -> C/C++ -> Preprocessor -> Preprocessor Definitions
// Add: ASFMLOGGER_HEADER_ONLY

// Project Properties -> C/C++ -> General -> Additional Include Directories
// Add: path/to/ASFMLogger
```

### GCC/Clang Project

```bash
# Compile with header-only ASFMLogger
g++ -std=c++17 -DASFMLOGGER_HEADER_ONLY -I/path/to/ASFMLogger -o MyApp main.cpp
```

## Feature Comparison

| Feature | Header-Only | Built Library |
|---------|-------------|---------------|
| Basic Logging | ✅ | ✅ |
| File Output | ✅ | ✅ |
| Console Output | ✅ | ✅ |
| Thread Safety | ✅ | ✅ |
| Error Handling | ✅ | ✅ |
| Configuration | ✅ | ✅ |
| Multi-Instance | ❌ | ✅ |
| Importance Framework | ❌ | ✅ |
| Database Integration | ❌ | ✅ |
| Shared Memory | ❌ | ✅ |
| Web Dashboard | ❌ | ✅ |
| Advanced Queue Management | ❌ | ✅ |

## Future Enhancements

### Potential Header-Only Improvements

1. **Template-based dispatch**: Reduce code duplication
2. **Conditional compilation**: Include only needed features
3. **External configuration**: JSON-based configuration loading
4. **Plugin architecture**: Load features dynamically

### Migration Path

The header-only approach provides a **smooth migration path** to the full built library:

```cpp
// Start with header-only for quick development
#define ASFMLOGGER_HEADER_ONLY
#include "ASFMLogger.hpp"

// Later, migrate to built library for production
// Remove define and link against ASFMLogger library
#include "ASFMLogger.hpp"
// target_link_libraries(MyApp ASFMLogger)
```

## Support

For header-only specific questions:
- **GitHub Issues**: https://github.com/AlotfyDev/ASFMLogger/issues
- **Documentation**: https://asfmlogger.dev/docs/header-only
- **Examples**: See `examples/header_only/` directory

---

**ASFMLogger Header-Only - Simple Integration, Powerful Logging**

The header-only approach makes ASFMLogger incredibly easy to integrate while maintaining the core logging functionality. Perfect for projects that need reliable logging without the complexity of building and linking libraries.