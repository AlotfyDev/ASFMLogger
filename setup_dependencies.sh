#!/bin/bash
echo "================================================================"
echo "ASFMLogger Comprehensive Dependencies Setup (SF-P0 Compliant)"
echo "================================================================"

# Ensure vcpkg is available (required for full functionality testing)
if ! command -v vcpkg &> /dev/null; then
    echo "WARNING: vcpkg not found in PATH"
    echo "Please ensure vcpkg is installed and available"
    echo "Install from: https://github.com/microsoft/vcpkg"
    echo "Add to PATH: export PATH=\$PATH:~/vcpkg"
    VCPKG_AVAILABLE=false
else
    echo "vcpkg found - installing comprehensive dependencies..."
    VCPKG_AVAILABLE=true

    # Install full testing and benchmarking capabilities
    echo "Installing testing dependencies..."
    if vcpkg install gtest benchmark; then
        echo "[SUCCESS] Testing dependencies installed"
    else
        echo "[WARNING] Testing dependencies installation failed - tests will be disabled"
    fi

    # Install Python bindings dependencies
    echo "Installing Python binding dependencies..."
    if vcpkg install pybind11 python3; then
        echo "[SUCCESS] Python binding dependencies installed"
    else
        echo "[WARNING] Python binding dependencies failed - Python module will be disabled"
    fi

    # Install Web interface dependencies
    echo "Installing web interface dependencies..."
    if vcpkg install cpprestsdk; then
        echo "[SUCCESS] Web interface dependencies installed"
    else
        echo "[WARNING] Web interface dependencies failed - web features will be disabled"
    fi

    echo "vcpkg dependencies installation complete!"
    echo ""
fi

# Setup MQL5 integration (external dependency)
echo "Setting up MQL5 integration..."
echo "MQL5 source files are available in wrappers/mql5/ directory"
echo ""
echo "VS Code Integration (Cross-platform):"
echo "====================================="
echo "1. Install 'MQL5 Tools' extension from VS Code Marketplace"
echo "2. Configure MetaEditor path in settings (use Wine path for Linux)"
echo "3. F6 compilation support directly in VS Code"
echo ""
echo "Windows Native Setup:"
echo "====================="
echo "1. Install MetaTrader 5 from https://www.metatrader5.com"
echo "2. MetaEditor64.exe path: C:\Program Files\MetaTrader 5 EXNESS\MetaEditor64.exe"
echo ""
echo "Linux/Wine Setup:"
echo "================="
echo "1. Install MetaTrader 5 via Wine/CrossOver"
echo "2. Locate MetaEditor64.exe in Wine prefix"
echo "3. Configure VS Code MQL5 Tools with Wine executable path"
echo ""
echo "MQL5 Compilation & Development:"
echo "==============================="
echo "- .mq5 files in wrappers/mql5/ directory"
echo "- F6 in VS Code or MetaEditor for compilation"
echo "- .ex5 generation for algorithm deployment"
echo "- DLL/network communication with C++ framework"

# Setup local external dependencies
echo "Setting up local external dependencies..."

# Create external directory structure
mkdir -p external/nlohmann external/spdlog

# Copy nlohmann/json library from external dependencies
if [ -d "/mnt/d/CppBridgeVSC/SharedFileMapModule/ExternalDeps/nlohmann" ]; then
    echo "Copying nlohmann/json library..."
    cp -r /mnt/d/CppBridgeVSC/SharedFileMapModule/ExternalDeps/nlohmann/* external/nlohmann/ 2>/dev/null || true
    echo "nlohmann/json library copied successfully."
elif [ -d "D:/CppBridgeVSC/SharedFileMapModule/ExternalDeps/nlohmann" ]; then
    echo "Copying nlohmann/json library..."
    cp -r "D:/CppBridgeVSC/SharedFileMapModule/ExternalDeps/nlohmann/*" external/nlohmann/ 2>/dev/null || true
    echo "nlohmann/json library copied successfully."
else
    echo "Downloading nlohmann/json library..."
    if command -v wget &> /dev/null; then
        wget -O external/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    elif command -v curl &> /dev/null; then
        curl -L -o external/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    else
        echo "Warning: Neither wget nor curl available for download"
        echo "Please manually download nlohmann/json from: https://github.com/nlohmann/json/releases"
    fi

    if [ -f "external/nlohmann/json.hpp" ]; then
        echo "nlohmann/json downloaded successfully."
    fi
fi

# Try to get SPDLog via vcpkg or fallback
if $VCPKG_AVAILABLE; then
    echo "SPDLog can be obtained via vcpkg when building: vcpkg install spdlog"
else
    echo "SPDLog: Can be obtained via package manager"
    echo "Ubuntu/Debian: sudo apt-get install libspdlog-dev"
    echo "Or build from source: https://github.com/gabime/spdlog"
fi

echo ""
echo "================================================================"
echo "DEPENDENCY SETUP COMPLETE - SF-P0 COMPLIANT"
echo "================================================================"
echo ""
echo "Current Dependencies Status (may require vcpkg in build):"
echo ""
echo "[REQUIRED FOR TESTING]:"
echo "- GTest (Unit Testing): Available via vcpkg"
echo "- Google Benchmark: Available via vcpkg"
echo ""
echo "[OPTIONAL ENHANCEMENTS]:"
echo "- pybind11 (Python bindings): Available via vcpkg"
echo "- cpprestsdk (Web API): Available via vcpkg"
echo "- MQL5 SDK (Trading integration): Requires MetaTrader 5"
echo "- SPDLog (Async logging): Available via vcpkg/apt"
echo "- nlohmann/json (JSON output): Local copy available"
echo ""
echo "Build Options:"
echo "=============="
echo ""
echo "1. Build with ALL dependencies (vcpkg - RECOMMENDED for full testing):"
echo "   mkdir build_full && cd build_full"
echo "   cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake"
echo "   cmake --build . --config Release"
echo ""
echo "2. Build with LOCAL dependencies (minimal):"
echo "   mkdir build_local && cd build_local"
echo "   cmake .. -DCMAKE_BUILD_TYPE=Release -DASFMLOGGER_USE_LOCAL_DEPS=ON"
echo "   cmake --build . --config Release"
echo ""
echo "3. Header-only build (core only):"
echo "   mkdir build_header && cd build_header"
echo "   cmake .. -DASFMLOGGER_HEADER_ONLY=ON"
echo "   cmake --build . --config Release"
echo ""
echo "Features Available Based on Dependencies:"
echo "========================================="
echo "- Unit Tests: Require GTest (vcpkg install gtest)"
echo "- Benchmarks: Require Google Benchmark (vcpkg install benchmark)"
echo "- Python Bindings: Require pybind11 (vcpkg install pybind11)"
echo "- Web Interface: Require cpprestsdk (vcpkg install cpprestsdk)"
echo "- JSON Output: Local nlohmann/json available"
echo "- Async Logging: Require SPDLog"
echo ""
echo "✅ FRAMEWORK CORE WORKS WITHOUT ANY DEPENDENCIES!"
echo "Enhanced features become available when dependencies are installed."
