@echo off
echo ================================================================
echo ASFMLogger Comprehensive Dependencies Setup (SF-P0 Compliant)
echo ================================================================

REM Ensure vcpkg is available (required for full functionality testing)
echo Checking for vcpkg installation...
vcpkg version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: vcpkg not found in PATH
    echo Please ensure vcpkg is installed and available
    echo Install from: https://github.com/microsoft/vcpkg
    echo Add to PATH: set PATH=%%PATH%%;C:\vcpkg
    goto :external_deps
)

echo Installing vcpkg dependencies for full framework functionality...
echo This provides: GTest, Google Benchmark, pybind11, cpprestsdk
echo.

REM Install full testing and benchmarking capabilities
echo Installing testing dependencies...
vcpkg install --triplet=x64-windows gtest benchmark
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Testing dependencies installed
) else (
    echo [WARNING] Testing dependencies installation failed - tests will be disabled
)

REM Install Python bindings dependencies
echo Installing Python binding dependencies...
vcpkg install --triplet=x64-windows pybind11 python3
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Python binding dependencies installed
) else (
    echo [WARNING] Python binding dependencies failed - Python module will be disabled
)

REM Install Web interface dependencies
echo Installing web interface dependencies...
vcpkg install --triplet=x64-windows cpprestsdk
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Web interface dependencies installed
) else (
    echo [WARNING] Web interface dependencies failed - web features will be disabled
)

REM Setup external MQL5 SDK (separate from vcpkg)
echo Setting up MQL5 integration...
echo Note: MQL5 SDK requires MetaTrader 5 platform installation
echo MQL5 source files are available in wrappers\mql5\ directory
echo.
echo VS Code Integration (Recommended):
echo ================================
echo 1. Install 'MQL5 Tools' extension from VS Code Marketplace
echo 2. Configure MetaEditor path in VS Code settings
echo 3. Your detected path: C:\Program Files\MetaTrader 5 EXNESS\MetaEditor64.exe
echo.
echo Manual MQL5 Development:
echo ========================
echo 1. Install MetaTrader 5 from https://www.metatrader5.com
echo 2. Use MT5 built-in MetaEditor or VS Code extension
echo 3. Communication with C++ framework via DLL/network interface (if needed)
echo.
echo MQL5 Compilation:
echo =================
echo - .mq5 files in wrappers\mql5\ directory
echo - Use F6 in MetaEditor or VS Code MQL5 Tools extension
echo - Generated .ex5 files can interface with ASFMLogger.dll

:vcpkg_complete
echo vcpkg dependencies installation complete!
echo.

:external_deps
echo Setting up local external dependencies...

REM Copy nlohmann/json library from external dependencies
if exist "D:\CppBridgeVSC\SharedFileMapModule\ExternalDeps\nlohmann" (
    echo Copying nlohmann/json library...
    if not exist "external\nlohmann" mkdir external\nlohmann
    xcopy /E /I /Y "D:\CppBridgeVSC\SharedFileMapModule\ExternalDeps\nlohmann" "external\nlohmann"
    echo nlohmann/json library copied successfully.
) else (
    echo Downloading nlohmann/json library...
    curl -L -o external\nlohmann\json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    if %ERRORLEVEL% EQU 0 (
        echo nlohmann/json header downloaded successfully.
    ) else (
        echo Warning: Could not download nlohmann/json
        echo Manual download from: https://github.com/nlohmann/json/releases
    )
)

REM Copy SPDLog library from external dependencies
if exist "D:\Sources\C++Software\SPDLog" (
    echo Copying SPDLog library...
    if not exist "external\spdlog" mkdir external\spdlog
    xcopy /E /I /Y "D:\Sources\C++Software\SPDLog" "external\spdlog"
    echo SPDLog library copied successfully.
) else (
    echo SPDLog can be obtained via vcpkg: vcpkg install spdlog
)

echo.
echo ================================================================
echo DEPENDENCY SETUP COMPLETE - SF-P0 COMPLIANT
echo ================================================================
echo.
echo Current Dependencies Status:
echo.
echo [REQUIRED FOR TESTING]:
echo - GTest (Unit Testing): %GTEST_STATUS%
echo - Google Benchmark: %BENCHMARK_STATUS%
echo.
echo [OPTIONAL ENHANCEMENTS]:
echo - pybind11 (Python bindings): %PYBIND11_STATUS%
echo - cpprestsdk (Web API): %WEB_STATUS%
echo - MQL5 SDK (Trading integration): Requires MetaTrader 5
echo - SPDLog (Async logging): %SPDLOG_STATUS%
echo - nlohmann/json (JSON output): %JSON_STATUS%
echo.
echo Build Options:
echo ==============
echo.
echo 1. Build with ALL dependencies (vcpkg - RECOMMENDED):
echo    mkdir build_full && cd build_full
echo    cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
echo    cmake --build . --config Release
echo.
echo 2. Build with LOCAL dependencies (fallback):
echo    mkdir build_local && cd build_local
echo    cmake .. -DCMAKE_BUILD_TYPE=Release -DASFMLOGGER_USE_LOCAL_DEPS=ON
echo    cmake --build . --config Release
echo.
echo 3. Header-only build (minimal):
echo    mkdir build_header && cd build_header
echo    cmake .. -DASFMLOGGER_HEADER_ONLY=ON
echo    cmake --build . --config Release
echo.
echo Features Available Based on Dependencies:
echo =========================================
echo - Unit Tests: Require GTest
echo - Benchmarks: Require Google Benchmark
echo - Python Bindings: Require pybind11 + Python3
echo - Web Interface: Require cpprestsdk
echo - JSON/Async Logging: Require nlohmann/json + SPDLog
echo.
echo The framework CORE works without any dependencies!
