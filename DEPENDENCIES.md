# ASFMLogger Dependencies

This document explains how to set up the required dependencies for the ASFMLogger project.

## Dependency Management Strategy

ASFMLogger supports **two dependency management approaches**:

### Option 1: Local Dependencies (Recommended)
Self-contained dependencies included with the project for maximum portability.

### Option 2: vcpkg Package Manager
Automatic dependency resolution using [vcpkg](https://vcpkg.io/).

## Required Dependencies

### Core Dependencies (Always Required)
- **spdlog** - High-performance logging library
- **gtest** - Google Test framework for unit testing

### Optional Dependencies
- **nlohmann-json** - JSON parsing library (enables enhanced configuration features)
- **odbc** - SQL Server connectivity support (Windows only)

## Installation Options

---

## Option 1: Local Dependencies (Recommended)

### Quick Setup
```bash
# 1. Copy nlohmann library to project
setup_dependencies.bat

# 2. Build with local dependencies
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DASFMLOGGER_USE_LOCAL_DEPS=ON
cmake --build . --config Release
```

### Manual Setup
1. **Copy nlohmann/json library:**
   ```bash
   # Copy from existing external dependencies
   xcopy /E /I /Y "D:\CppBridgeVSC\SharedFileMapModule\ExternalDeps\nlohmann" "external\nlohmann"

   # OR download manually:
   # wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp -O external/nlohmann/json.hpp
   ```

2. **Build the project:**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DASFMLOGGER_USE_LOCAL_DEPS=ON
   cmake --build . --config Release
   ```

---

## Option 2: vcpkg Package Manager

### 1. Install vcpkg
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat  # Windows
./vcpkg integrate install
```

### 2. Install Dependencies
```bash
# Install core dependencies
vcpkg install spdlog gtest

# Install optional JSON support
vcpkg install nlohmann-json

# Install optional SQL Server support (Windows only)
vcpkg install odbc
```

### 3. Configure CMake
```cmake
# Set vcpkg toolchain (already configured in CMakeLists.txt)
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
```

### 4. Build the Project
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Local Dependencies Structure

When using local dependencies, the project expects the following structure:

```
ASFMLogger/
├── external/
│   ├── nlohmann/
│   │   └── json.hpp          # Single header file for JSON support
│   └── spdlog/
│       └── include/          # SPDLog header files directory
├── src/
│   └── toolbox/
│       ├── ConfigurationToolbox.hpp
│       └── ConfigurationToolbox.cpp
└── CMakeLists.txt
```

## Dependency Features

### JSON Support (Optional)
- **Library**: nlohmann/json (single header file)
- **Location**: `external/nlohmann/json.hpp`
- **Feature**: Configuration serialization/deserialization
- **Fallback**: String-based configuration when not available

**When nlohmann/json is available:**
- `ASFMLOGGER_JSON_SUPPORT` is defined
- Full JSON configuration parsing and serialization
- Enhanced configuration management features

**When nlohmann/json is not available:**
- Graceful fallback to string-based operations
- Core functionality remains intact

### SPDLog Support (Optional)
- **Library**: SPDLog (header-only library)
- **Location**: `external/spdlog/include/spdlog/`
- **Feature**: High-performance logging backend
- **Fallback**: Basic logging when not available

**When SPDLog is available:**
- `ASFMLOGGER_SPDLOG_SUPPORT` is defined
- Enhanced logging performance and features
- Advanced logging patterns and formatting

**When SPDLog is not available:**
- Basic logging functionality remains available
- Standard library logging used as fallback

### SQL Server Support (Optional)
- **Library**: ODBC drivers
- **Feature**: Database logging capabilities
- **Platform**: Windows only
- **Fallback**: File-based logging when not available

## Verification

After setup, verify that dependencies are properly detected:

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

You should see in the output:
```
ASFMLogger Configuration Summary:
  Version: 1.0.0
  C++ Standard: 17
  Build Type: Release
  GTest: Found via vcpkg
  SPDLog: Found via vcpkg             # (when using vcpkg)
  SPDLog: Found local copy            # (when using local deps)
  SPDLog: Not found                   # (when not available)
  nlohmann/json: Found local copy     # (when using local deps)
  nlohmann/json: Found via vcpkg      # (when using vcpkg)
  nlohmann/json: Not found            # (when not available)
  SQL Server Support: Found via vcpkg # (if available)
```

## Project Structure with Dependencies

```
ASFMLogger/
├── external/                    # Local dependencies
│   ├── nlohmann/
│   │   ├── json.hpp            # JSON library header
│   │   └── README.md           # Library documentation
│   └── spdlog/
│       ├── include/            # SPDLog header files
│       │   └── spdlog/         # SPDLog headers
│       └── README.md           # SPDLog documentation
├── src/                        # Source code
├── tests/                      # Test files
├── examples/                   # Example applications
├── docs/                       # Documentation
├── CMakeLists.txt             # Build configuration
├── vcpkg.json                 # vcpkg dependencies (optional)
├── setup_dependencies.bat      # Windows dependency setup script
├── setup_dependencies.sh       # Linux dependency setup script
└── DEPENDENCIES.md            # This file
```

## Troubleshooting

### Common Issues

1. **Local nlohmann/json not found**
   ```bash
   # Run the setup script
   setup_dependencies.bat

   # Or manually copy the file
   xcopy "D:\CppBridgeVSC\SharedFileMapModule\ExternalDeps\nlohmann\*" "external\nlohmann\"
   ```

2. **vcpkg not found**
   ```bash
   # Make sure vcpkg is properly installed and integrated
   ./vcpkg integrate install
   ```

3. **CMake cannot find dependencies**
   ```cmake
   # For local dependencies:
   cmake .. -DASFMLOGGER_USE_LOCAL_DEPS=ON

   # For vcpkg dependencies:
   cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
   ```

4. **ODBC not found (Windows)**
   ```bash
   # Install ODBC drivers and development headers
   vcpkg install odbc
   ```

### Platform-Specific Notes

#### Windows
- **Local Dependencies**: Copy files to `external/` directory
- **vcpkg**: Ensure Visual Studio 2019+ is installed
- **ODBC**: Requires Windows SDK

#### Linux
- **Local Dependencies**: Single header file approach recommended
- **vcpkg**: Works with GCC 7+ or Clang 5+
- **ODBC**: Varies by distribution

#### macOS
- **Local Dependencies**: Single header file approach recommended
- **vcpkg**: Requires Xcode command line tools
- **ODBC**: Limited support

## Support

For dependency-related issues:
1. **Local Dependencies**: Check that files exist in `external/` directory
2. **vcpkg Dependencies**: Verify installation via `vcpkg list`
3. **CMake Configuration**: Check build output for dependency status
4. **Consult Documentation**: [vcpkg docs](https://vcpkg.io/) for vcpkg issues
5. **GitHub Issues**: [ASFMLogger Issues](https://github.com/your-repo/asfmlogger/issues) for project-specific problems

## Recommendation

**For Development**: Use local dependencies for simplicity and portability
**For Production**: Use vcpkg for automatic dependency management and versioning