@echo off
REM ASFMLogger Package Creation Script for Windows
REM This script creates distributable packages for ASFMLogger

setlocal enabledelayedexpansion

echo ASFMLogger Package Creation
echo ===========================

REM Check if build directory exists
if not exist "build" (
    echo Error: Build directory not found. Please build the project first.
    echo Run: cmake --build build --config Release
    pause
    exit /b 1
)

REM Create packages directory
if not exist "packages" mkdir packages

set PACKAGE_VERSION=1.0.0
set PACKAGE_NAME=ASFMLogger-%PACKAGE_VERSION%-Windows-x64

echo Creating package: %PACKAGE_NAME%

REM Create package directory structure
set PACKAGE_DIR=packages\%PACKAGE_NAME%
if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
mkdir "%PACKAGE_DIR%"
mkdir "%PACKAGE_DIR%\bin"
mkdir "%PACKAGE_DIR%\lib"
mkdir "%PACKAGE_DIR%\include"
mkdir "%PACKAGE_DIR%\config"
mkdir "%PACKAGE_DIR%\docs"
mkdir "%PACKAGE_DIR%\wrappers"
mkdir "%PACKAGE_DIR%\examples"

REM Copy built binaries
echo Copying binaries...
copy "build\bin\*.dll" "%PACKAGE_DIR%\bin\" >nul 2>&1
copy "build\bin\*.exe" "%PACKAGE_DIR%\bin\" >nul 2>&1
copy "build\lib\*.lib" "%PACKAGE_DIR%\lib\" >nul 2>&1

REM Copy headers
echo Copying headers...
xcopy "ASFMLogger.hpp" "%PACKAGE_DIR%\include\" /y >nul 2>&1
xcopy "src" "%PACKAGE_DIR%\include\" /s /y /exclude:exclude_src.txt >nul 2>&1

REM Copy configuration files
echo Copying configuration...
copy "deploy\production.config.json" "%PACKAGE_DIR%\config\" >nul 2>&1
copy "deploy\install_service.bat" "%PACKAGE_DIR%\" >nul 2>&1

REM Copy documentation
echo Copying documentation...
copy "README.md" "%PACKAGE_DIR%\docs\" >nul 2>&1
copy "docs\*.md" "%PACKAGE_DIR%\docs\" >nul 2>&1

REM Copy wrappers
echo Copying wrappers...
xcopy "wrappers" "%PACKAGE_DIR%\wrappers\" /s /y >nul 2>&1

REM Copy examples (if they exist)
if exist "examples" (
    echo Copying examples...
    xcopy "examples" "%PACKAGE_DIR%\examples\" /s /y >nul 2>&1
)

REM Create CMake package configuration
echo Creating CMake configuration...
mkdir "%PACKAGE_DIR%\lib\cmake\ASFMLogger"
copy "build\ASFMLoggerConfig.cmake" "%PACKAGE_DIR%\lib\cmake\ASFMLogger\" >nul 2>&1
copy "build\ASFMLoggerConfigVersion.cmake" "%PACKAGE_DIR%\lib\cmake\ASFMLogger\" >nul 2>&1

REM Create package information
echo Creating package information...
(
echo ASFMLogger v%PACKAGE_VERSION%
echo ===============================
echo.
echo Description: Enterprise-grade logging framework with multi-language support
echo Platform: Windows x64
echo Build Type: Release
echo Build Date: %DATE% %TIME%
echo.
echo Features:
echo - Multi-instance logging with application tracking
echo - Smart message classification with importance framework
echo - Contextual persistence with adaptive policies
echo - SQL Server integration for enterprise database logging
echo - Windows FileMaps support for inter-process communication
echo - Real-time monitoring with web dashboard
echo - Multi-language support ^(C++, Python, C#, MQL5^)
echo.
echo Installation:
echo 1. Extract to your desired location
echo 2. Run install_service.bat as Administrator to install as Windows service
echo 3. Configure production.config.json for your environment
echo 4. Start using ASFMLogger in your applications
echo.
echo Documentation: docs\
echo Examples: examples\
echo Wrappers: wrappers\
) > "%PACKAGE_DIR%\README.txt"

REM Create ZIP package
echo Creating ZIP archive...
powershell -Command "Add-Type -Assembly 'System.IO.Compression.FileSystem'; [System.IO.Compression.ZipFile]::CreateFromDirectory('%PACKAGE_DIR%', 'packages\%PACKAGE_NAME%.zip')"

REM Create MSI installer (optional, requires WiX Toolset)
if exist "C:\Program Files (x86)\WiX Toolset v3.11\bin\heat.exe" (
    echo Creating MSI installer...
    "C:\Program Files (x86)\WiX Toolset v3.11\bin\heat.exe" dir "%PACKAGE_DIR%" -out "%PACKAGE_DIR%\files.wxs" -gg -scom -sreg -sfrag -srd -dr INSTALLDIR -cg ASFMLoggerComponents -var var.SourceDir
    "C:\Program Files (x86)\WiX Toolset v3.11\bin\candle.exe" "%PACKAGE_DIR%\product.wxs" "%PACKAGE_DIR%\files.wxs" -dSourceDir="%PACKAGE_DIR%"
    "C:\Program Files (x86)\WiX Toolset v3.11\bin\light.exe" "%PACKAGE_DIR%\product.wxobj" "%PACKAGE_DIR%\files.wxobj" -out "packages\%PACKAGE_NAME%.msi"
)

REM Create NuGet package for C# wrapper
if exist "wrappers\csharp\ASFMLogger.Wrapper.csproj" (
    echo Creating NuGet package...
    cd wrappers\csharp
    dotnet pack --configuration Release --output "..\..\packages"
    cd ..\..
)

REM Create Python wheel
if exist "wrappers\python\setup.py" (
    echo Creating Python wheel...
    cd wrappers\python
    python setup.py bdist_wheel --dist-dir "..\..\packages"
    cd ..\..
)

REM Calculate package size and verify
echo Verifying package...
for %%f in ("packages\%PACKAGE_NAME%.zip") do set PACKAGE_SIZE=%%~zf
echo Package created successfully:
echo   Location: packages\%PACKAGE_NAME%.zip
echo   Size: !PACKAGE_SIZE! bytes
if exist "packages\%PACKAGE_NAME%.msi" (
    for %%f in ("packages\%PACKAGE_NAME%.msi") do set MSI_SIZE=%%~zf
    echo   MSI Installer: packages\%PACKAGE_NAME%.msi ^(!MSI_SIZE! bytes^)
)

echo.
echo Package contents:
dir "%PACKAGE_DIR%" /s /b | findstr /v /c:"\\build\\"

echo.
echo Package creation completed successfully!
echo.
echo To install:
echo 1. Extract %PACKAGE_NAME%.zip to your desired location
echo 2. Run install_service.bat as Administrator
echo 3. Configure production.config.json
echo 4. Start the service
echo.
echo For detailed instructions, see docs\DEPLOYMENT_GUIDE.md

pause