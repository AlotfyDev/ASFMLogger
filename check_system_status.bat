@echo off
echo ========================================
echo ASFMLogger System Status Check
echo ========================================
echo.

echo 1. Checking vcpkg installation...
echo ========================================
vcpkg version 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ vcpkg NOT FOUND
    echo Please install from: https://github.com/microsoft/vcpkg
) else (
    echo ✅ vcpkg FOUND
    vcpkg version | findstr "version"
)
echo.

echo 2. Checking vcpkg integration...
echo ==================================
vcpkg integrate list 2>nul | findstr "integration"
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Integration NOT CONFIGURED
) else (
    echo ✅ Integration CONFIGURED
)
echo.

echo 3. Checking required packages...
echo ================================
echo GTest packages:
vcpkg list 2>nul | findstr gtest || echo ❌ gtest NOT INSTALLED
echo.

echo Benchmark packages:
vcpkg list 2>nul | findstr benchmark || echo ❌ benchmark NOT INSTALLED
echo.

echo Python binding packages:
vcpkg list 2>nul | findstr pybind11 || echo ❌ pybind11 NOT INSTALLED
vcpkg list 2>nul | findstr python3 || echo ❌ python3 NOT INSTALLED
echo.

echo Web interface packages:
vcpkg list 2>nul | findstr cpprestsdk || echo ❌ cpprestsdk NOT INSTALLED
echo.

echo Logging packages:
vcpkg list 2>nul | findstr spdlog || echo ❌ spdlog NOT INSTALLED
echo.

echo 4. MetaTrader 5 check...
echo ========================
if exist "C:\Program Files\MetaTrader 5 EXNESS\MetaEditor64.exe" (
    echo ✅ MetaTrader 5 FOUND at C:\Program Files\MetaTrader 5 EXNESS\
) else (
    echo ❌ MetaTrader 5 NOT FOUND at expected location
    echo Please install from: https://www.metatrader5.com
)
echo.

echo 5. VS Code extensions check (manual)...
echo ======================================
echo ⚠️  Please manually check VS Code for:
echo    - "C/C++" extension (Microsoft)
echo    - "CMake Tools" extension (Microsoft) 
echo    - "MQL5 Tools" extension
echo.

echo ========================================
echo SUMMARY & NEXT STEPS
echo ========================================
echo.
echo If all checks show ✅, run: setup_dependencies.bat
echo.
echo If issues found, please share this output for troubleshooting.
echo.
pause
