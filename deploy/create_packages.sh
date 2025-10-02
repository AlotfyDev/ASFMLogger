#!/bin/bash

# ASFMLogger Package Creation Script for Linux/macOS
# This script creates distributable packages for ASFMLogger

set -e

echo "ASFMLogger Package Creation"
echo "==========================="

# Check if build directory exists
if [[ ! -d "build" ]]; then
    echo "Error: Build directory not found. Please build the project first."
    echo "Run: cmake --build build --config Release"
    exit 1
fi

# Create packages directory
mkdir -p packages

PACKAGE_VERSION="1.0.0"
PACKAGE_NAME="ASFMLogger-${PACKAGE_VERSION}-Linux-x64"

echo "Creating package: $PACKAGE_NAME"

# Create package directory structure
PACKAGE_DIR="packages/$PACKAGE_NAME"
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/bin"
mkdir -p "$PACKAGE_DIR/lib"
mkdir -p "$PACKAGE_DIR/include"
mkdir -p "$PACKAGE_DIR/config"
mkdir -p "$PACKAGE_DIR/docs"
mkdir -p "$PACKAGE_DIR/wrappers"
mkdir -p "$PACKAGE_DIR/examples"

# Copy built binaries
echo "Copying binaries..."
if [[ -f "build/bin/libASFMLogger.so" ]]; then
    cp "build/bin/libASFMLogger.so" "$PACKAGE_DIR/lib/"
elif [[ -f "build/bin/libASFMLogger.dylib" ]]; then
    cp "build/bin/libASFMLogger.dylib" "$PACKAGE_DIR/lib/"
elif [[ -f "build/bin/ASFMLogger.dll" ]]; then
    cp "build/bin/ASFMLogger.dll" "$PACKAGE_DIR/bin/"
fi

# Copy headers
echo "Copying headers..."
cp ASFMLogger.hpp "$PACKAGE_DIR/include/"
cp -r src/* "$PACKAGE_DIR/include/"

# Copy configuration files
echo "Copying configuration..."
cp deploy/production.config.json "$PACKAGE_DIR/config/"
cp deploy/install_service.sh "$PACKAGE_DIR/"

# Copy documentation
echo "Copying documentation..."
cp README.md "$PACKAGE_DIR/docs/"
cp docs/*.md "$PACKAGE_DIR/docs/"

# Copy wrappers
echo "Copying wrappers..."
cp -r wrappers/* "$PACKAGE_DIR/wrappers/"

# Copy examples (if they exist)
if [[ -d "examples" ]]; then
    echo "Copying examples..."
    cp -r examples/* "$PACKAGE_DIR/examples/"
fi

# Create CMake package configuration
echo "Creating CMake configuration..."
mkdir -p "$PACKAGE_DIR/lib/cmake/ASFMLogger"
cp build/ASFMLoggerConfig.cmake "$PACKAGE_DIR/lib/cmake/ASFMLogger/" 2>/dev/null || true
cp build/ASFMLoggerConfigVersion.cmake "$PACKAGE_DIR/lib/cmake/ASFMLogger/" 2>/dev/null || true

# Create package information
echo "Creating package information..."
cat > "$PACKAGE_DIR/README.txt" << EOF
ASFMLogger v$PACKAGE_VERSION
===============================

Description: Enterprise-grade logging framework with multi-language support
Platform: Linux x64
Build Type: Release
Build Date: $(date)

Features:
- Multi-instance logging with application tracking
- Smart message classification with importance framework
- Contextual persistence with adaptive policies
- SQL Server integration for enterprise database logging
- Shared memory support for inter-process communication
- Real-time monitoring with web dashboard
- Multi-language support (C++, Python, C#, MQL5)

Installation:
1. Extract to your desired location
2. Run install_service.sh as root to install as systemd service
3. Configure config/production.json for your environment
4. Start using ASFMLogger in your applications

Documentation: docs/
Examples: examples/
Wrappers: wrappers/

For detailed instructions, see docs/DEPLOYMENT_GUIDE.md
EOF

# Create tar.gz package
echo "Creating tar.gz archive..."
cd packages
tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
cd ..

# Create Debian package (if dpkg available)
if command -v dpkg &> /dev/null; then
    echo "Creating Debian package..."

    # Create DEBIAN control file
    mkdir -p "$PACKAGE_DIR/DEBIAN"

    cat > "$PACKAGE_DIR/DEBIAN/control" << EOF
Package: asfmlogger
Version: $PACKAGE_VERSION
Section: devel
Priority: optional
Architecture: amd64
Depends: libc6 (>= 2.27), libstdc++6 (>= 8.0), libgcc1 (>= 8.0)
Recommends: spdlog, libodbc1
Maintainer: ASFMLogger Development Team <support@asfmlogger.dev>
Description: Enterprise-grade logging framework with multi-language support
 ASFMLogger (Abstract Shared File Map Logger) is a comprehensive,
 enterprise-grade, multi-language logging framework built on advanced
 architectural principles for modern distributed applications.
 .
 Features include multi-instance logging, smart message classification,
 contextual persistence, SQL Server integration, shared memory support,
 real-time monitoring, and multi-language wrappers for C++, Python, C#, and MQL5.
Homepage: https://github.com/AlotfyDev/ASFMLogger
EOF

    # Set permissions
    chmod 755 "$PACKAGE_DIR/DEBIAN"
    chmod 644 "$PACKAGE_DIR/DEBIAN/control"

    # Build Debian package
    dpkg-deb --build "$PACKAGE_DIR" "packages/${PACKAGE_NAME}.deb"

    echo "Debian package created: packages/${PACKAGE_NAME}.deb"
fi

# Create RPM package (if rpmbuild available)
if command -v rpmbuild &> /dev/null; then
    echo "Creating RPM package..."

    # Create RPM spec file
    cat > "packages/${PACKAGE_NAME}.spec" << EOF
Name:       asfmlogger
Version:    $PACKAGE_VERSION
Release:    1
Summary:    Enterprise-grade logging framework
License:    MIT
URL:        https://github.com/AlotfyDev/ASFMLogger

%description
ASFMLogger (Abstract Shared File Map Logger) is a comprehensive,
enterprise-grade, multi-language logging framework built on advanced
architectural principles for modern distributed applications.

%install
mkdir -p %{buildroot}/opt/asfmlogger
cp -r $PACKAGE_DIR/* %{buildroot}/opt/asfmlogger/

%files
/opt/asfmlogger
EOF

    # Build RPM package
    rpmbuild -bb "packages/${PACKAGE_NAME}.spec" --buildroot "$(pwd)/packages/rpmbuild"

    echo "RPM package created: packages/${PACKAGE_NAME}.rpm"
fi

# Calculate package sizes
echo "Verifying packages..."
cd packages

if [[ -f "${PACKAGE_NAME}.tar.gz" ]]; then
    TAR_SIZE=$(stat -c%s "${PACKAGE_NAME}.tar.gz" 2>/dev/null || echo 0)
    echo "Tar.gz package: ${PACKAGE_NAME}.tar.gz (${TAR_SIZE} bytes)"
fi

if [[ -f "${PACKAGE_NAME}.deb" ]]; then
    DEB_SIZE=$(stat -c%s "${PACKAGE_NAME}.deb" 2>/dev/null || echo 0)
    echo "Debian package: ${PACKAGE_NAME}.deb (${DEB_SIZE} bytes)"
fi

if [[ -f "${PACKAGE_NAME}.rpm" ]]; then
    RPM_SIZE=$(stat -c%s "${PACKAGE_NAME}.rpm" 2>/dev/null || echo 0)
    echo "RPM package: ${PACKAGE_NAME}.rpm (${RPM_SIZE} bytes)"
fi

cd ..

echo ""
echo "Package contents:"
find "$PACKAGE_DIR" -type f | head -20

echo ""
echo "Package creation completed successfully!"
echo ""
echo "To install:"
echo "  Tar.gz:  tar -xzf packages/${PACKAGE_NAME}.tar.gz -C /opt/"
echo "  Debian:  sudo dpkg -i packages/${PACKAGE_NAME}.deb"
echo "  RPM:     sudo rpm -i packages/${PACKAGE_NAME}.rpm"
echo ""
echo "For detailed instructions, see docs/DEPLOYMENT_GUIDE.md"