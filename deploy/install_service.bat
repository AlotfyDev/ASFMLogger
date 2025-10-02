@echo off
REM ASFMLogger Windows Service Installation Script
REM This script installs ASFMLogger as a Windows service

setlocal enabledelayedexpansion

echo ASFMLogger Service Installation
echo ===============================

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running with administrator privileges
) else (
    echo Requesting administrator privileges...
    powershell "start-process '%~f0' -verb runas"
    exit /b
)

REM Set service configuration
set SERVICE_NAME=ASFMLoggerService
set SERVICE_DISPLAY_NAME=ASFMLogger Enterprise Logging Service
set SERVICE_DESCRIPTION=Enterprise-grade logging service with multi-language support
set SERVICE_EXECUTABLE=%~dp0bin\ASFMLoggerService.exe
set SERVICE_CONFIG=%~dp0production.config.json

REM Check if service executable exists
if not exist "%SERVICE_EXECUTABLE%" (
    echo Error: Service executable not found at %SERVICE_EXECUTABLE%
    echo Please build the project first.
    pause
    exit /b 1
)

REM Check if configuration file exists
if not exist "%SERVICE_CONFIG%" (
    echo Warning: Configuration file not found at %SERVICE_CONFIG%
    echo Using default configuration.
)

echo Installing ASFMLogger service...

REM Create service using sc command
sc create "%SERVICE_NAME%" binPath= "\"%SERVICE_EXECUTABLE%\" --config \"%SERVICE_CONFIG%\"" start= auto DisplayName= "%SERVICE_DISPLAY_NAME%" depend= Tcpip/LanmanWorkstation

if %errorLevel% equ 0 (
    echo Service installed successfully!

    REM Set service description
    sc description "%SERVICE_NAME%" "%SERVICE_DESCRIPTION%"

    echo Configuring service settings...

    REM Configure service recovery options
    sc failure "%SERVICE_NAME%" reset= 3600 actions= restart/60000/restart/60000/restart/60000

    echo Service configuration completed.
    echo.
    echo Service Information:
    echo   Name: %SERVICE_NAME%
    echo   Display Name: %SERVICE_DISPLAY_NAME%
    echo   Startup: Automatic
    echo   Configuration: %SERVICE_CONFIG%
    echo.
    echo You can start the service with: net start %SERVICE_NAME%
    echo You can stop the service with: net stop %SERVICE_NAME%

) else (
    echo Error: Failed to install service. Error code: %errorLevel%
    echo Please check the Windows Event Viewer for more details.
)

echo.
echo Installation completed.
pause