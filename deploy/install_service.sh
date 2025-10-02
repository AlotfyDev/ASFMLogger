#!/bin/bash

# ASFMLogger Linux/macOS Service Installation Script
# This script installs ASFMLogger as a systemd service (Linux) or launchd service (macOS)

set -e

echo "ASFMLogger Service Installation"
echo "==============================="

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    SERVICE_TYPE="systemd"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
    SERVICE_TYPE="launchd"
else
    echo "Error: Unsupported operating system: $OSTYPE"
    exit 1
fi

echo "Detected OS: $OS ($SERVICE_TYPE)"

# Check if running as root (required for service installation)
if [[ $EUID -eq 0 ]]; then
    echo "Running with root privileges"
else
    echo "This script requires root privileges for service installation."
    echo "Please run with sudo or as root user."
    exit 1
fi

# Configuration
SERVICE_NAME="asfmlogger"
SERVICE_DISPLAY_NAME="ASFMLogger Enterprise Logging Service"
SERVICE_DESCRIPTION="Enterprise-grade logging service with multi-language support"
SERVICE_EXECUTABLE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/bin/ASFMLoggerService"
SERVICE_CONFIG="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/production.config.json"
INSTALL_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check if service executable exists
if [[ ! -f "$SERVICE_EXECUTABLE" ]]; then
    echo "Error: Service executable not found at $SERVICE_EXECUTABLE"
    echo "Please build the project first."
    exit 1
fi

# Check if configuration file exists
if [[ ! -f "$SERVICE_CONFIG" ]]; then
    echo "Warning: Configuration file not found at $SERVICE_CONFIG"
    echo "Using default configuration."
fi

echo "Installing ASFMLogger service..."

# Install service based on OS
if [[ "$OS" == "linux" ]]; then
    # Create systemd service file
    cat > "/etc/systemd/system/${SERVICE_NAME}.service" << EOF
[Unit]
Description=${SERVICE_DISPLAY_NAME}
Documentation=https://github.com/AlotfyDev/ASFMLogger
After=network.target

[Service]
Type=simple
ExecStart="${SERVICE_EXECUTABLE}" --config "${SERVICE_CONFIG}"
Restart=always
RestartSec=5
User=asfmlogger
Group=asfmlogger
Environment=ASPMNETCORE_ENVIRONMENT=Production
WorkingDirectory=${INSTALL_DIR}

# Security settings
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=${INSTALL_DIR}/logs ${INSTALL_DIR}/data

# Resource limits
LimitNOFILE=65536
MemoryLimit=100M

[Install]
WantedBy=multi-user.target
EOF

    # Create service user
    if ! id -u asfmlogger > /dev/null 2>&1; then
        useradd --system --shell /bin/false --home ${INSTALL_DIR} asfmlogger
    fi

    # Set ownership
    chown -R asfmlogger:asfmlogger ${INSTALL_DIR}

    # Reload systemd
    systemctl daemon-reload

    echo "Service installed successfully!"

elif [[ "$OS" == "macos" ]]; then
    # Create launchd plist file
    cat > "/Library/LaunchDaemons/${SERVICE_NAME}.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>${SERVICE_NAME}</string>
    <key>ProgramArguments</key>
    <array>
        <string>${SERVICE_EXECUTABLE}</string>
        <string>--config</string>
        <string>${SERVICE_CONFIG}</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
    <key>StandardOutPath</key>
    <string>${INSTALL_DIR}/logs/service.out.log</string>
    <key>StandardErrorPath</key>
    <string>${INSTALL_DIR}/logs/service.err.log</string>
    <key>WorkingDirectory</key>
    <string>${INSTALL_DIR}</string>
    <key>UserName</key>
    <string>asfmlogger</string>
    <key>GroupName</key>
    <string>asfmlogger</string>
    <key>EnvironmentVariables</key>
    <dict>
        <key>ASPNETCORE_ENVIRONMENT</key>
        <string>Production</string>
    </dict>
</dict>
</plist>
EOF

    # Create service user (macOS)
    if ! dscl . -read /Users/asfmlogger > /dev/null 2>&1; then
        dscl . -create /Users/asfmlogger
        dscl . -create /Users/asfmlogger UserShell /bin/false
        dscl . -create /Users/asfmlogger RealName "ASFMLogger Service User"
        dscl . -create /Users/asfmlogger UniqueID 666
        dscl . -create /Users/asfmlogger PrimaryGroupID 666
        dscl . -create /Users/asfmlogger NFSHomeDirectory ${INSTALL_DIR}
    fi

    # Set ownership
    chown -R asfmlogger:asfmlogger ${INSTALL_DIR}

    # Load the service
    launchctl load "/Library/LaunchDaemons/${SERVICE_NAME}.plist"

    echo "Service installed successfully!"
fi

echo ""
echo "Service Information:"
echo "  Name: $SERVICE_NAME"
echo "  Display Name: $SERVICE_DISPLAY_NAME"
echo "  Configuration: $SERVICE_CONFIG"
echo "  Install Directory: $INSTALL_DIR"
echo ""

if [[ "$OS" == "linux" ]]; then
    echo "Service Management Commands:"
    echo "  Start:  systemctl start $SERVICE_NAME"
    echo "  Stop:   systemctl stop $SERVICE_NAME"
    echo "  Status: systemctl status $SERVICE_NAME"
    echo "  Enable: systemctl enable $SERVICE_NAME"
    echo "  Logs:   journalctl -u $SERVICE_NAME -f"
elif [[ "$OS" == "macos" ]]; then
    echo "Service Management Commands:"
    echo "  Start:  launchctl start $SERVICE_NAME"
    echo "  Stop:   launchctl stop $SERVICE_NAME"
    echo "  Status: launchctl list | grep $SERVICE_NAME"
    echo "  Logs:   tail -f ${INSTALL_DIR}/logs/service.out.log"
fi

echo ""
echo "Installation completed successfully!"