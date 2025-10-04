# ASFMLogger Deployment Guide

## Overview

This guide provides comprehensive instructions for deploying ASFMLogger in various environments, from development to production. ASFMLogger supports multiple deployment scenarios including Windows services, Linux systemd services, Docker containers, and cloud deployments.

## Quick Start

### Prerequisites

- **C++17** or higher compiler
- **CMake** 3.20 or higher
- **vcpkg** (recommended for dependency management)
- **Optional**: SQL Server (for database logging)
- **Optional**: Web server framework (for REST API)

### Basic Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/AlotfyDev/ASFMLogger.git
   cd ASFMLogger
   ```

2. **Install dependencies with vcpkg:**
   ```bash
   vcpkg install gtest spdlog --triplet x64-windows
   vcpkg install gtest spdlog fmt --triplet x64-linux
   ```

3. **Configure and build:**
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build build --config Release
   ```

4. **Install:**
   ```bash
   cmake --install build --prefix "C:/ASFMLogger"
   ```

## Deployment Scenarios

### 1. Windows Service Deployment

#### Automatic Installation
```batch
# Run as Administrator
deploy\install_service.bat
```

#### Manual Installation
```batch
# Create service
sc create ASFMLoggerService binPath= "C:\ASFMLogger\bin\ASFMLoggerService.exe --config C:\ASFMLogger\deploy\production.config.json" start= auto

# Set description
sc description ASFMLoggerService "ASFMLogger Enterprise Logging Service"

# Configure recovery
sc failure ASFMLoggerService reset= 3600 actions= restart/60000/restart/60000/restart/60000

# Start service
net start ASFMLoggerService
```

#### Service Management
```batch
# Start service
net start ASFMLoggerService

# Stop service
net stop ASFMLoggerService

# Check status
sc query ASFMLoggerService

# View logs
eventvwr.msc
```

### 2. Linux Systemd Deployment

#### Automatic Installation
```bash
# Run as root
sudo deploy/install_service.sh
```

#### Manual Installation
```bash
# Copy service file
sudo cp deploy/asfmlogger.service /etc/systemd/system/

# Create service user
sudo useradd --system --shell /bin/false --home /opt/asfmlogger asfmlogger

# Set permissions
sudo chown -R asfmlogger:asfmlogger /opt/asfmlogger

# Reload systemd
sudo systemctl daemon-reload

# Enable and start service
sudo systemctl enable asfmlogger
sudo systemctl start asfmlogger
```

#### Service Management
```bash
# Start service
sudo systemctl start asfmlogger

# Stop service
sudo systemctl stop asfmlogger

# Check status
sudo systemctl status asfmlogger

# View logs
sudo journalctl -u asfmlogger -f

# Restart service
sudo systemctl restart asfmlogger
```

### 3. Docker Deployment

#### Dockerfile
```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libspdlog-dev \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy ASFMLogger source
COPY . /asfmlogger
WORKDIR /asfmlogger

# Build ASFMLogger
RUN mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make -j$(nproc) \
    && make install

# Create runtime user
RUN useradd --system --shell /bin/false asfmlogger

# Set ownership
RUN chown -R asfmlogger:asfmlogger /opt/asfmlogger

# Switch to non-root user
USER asfmlogger

# Expose ports for web dashboard
EXPOSE 8080

# Start ASFMLogger service
CMD ["/opt/asfmlogger/bin/ASFMLoggerService", "--config", "/opt/asfmlogger/config/production.json"]
```

#### Docker Compose
```yaml
version: '3.8'
services:
  asfmlogger:
    build: .
    ports:
      - "8080:8080"
    volumes:
      - ./config:/opt/asfmlogger/config:ro
      - ./logs:/opt/asfmlogger/logs
      - ./data:/opt/asfmlogger/data
    environment:
      - ASPNETCORE_ENVIRONMENT=Production
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/api/health"]
      interval: 30s
      timeout: 10s
      retries: 3
```

### 4. Cloud Deployment

#### AWS EC2 Deployment
```bash
# Launch EC2 instance with Windows Server 2019/2022
# Install dependencies
vcpkg install gtest spdlog --triplet x64-windows

# Build and deploy
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix "C:/ASFMLogger"

# Install as Windows service
deploy\install_service.bat
```

#### Azure App Service
```bash
# For web dashboard deployment
az webapp create --resource-group myResourceGroup --plan myPlan --name myASFMLogger --runtime "DOTNET|6.0"

# Deploy web dashboard
az webapp deployment source config --name myASFMLogger --resource-group myResourceGroup \
    --repo-url https://github.com/AlotfyDev/ASFMLogger --branch main --manual-integration

# Configure environment variables
az webapp config appsettings set --name myASFMLogger --resource-group myResourceGroup \
    --setting-names DATABASE_CONNECTION_STRING="Server=sql-server;Database=logs;..."
```

## Configuration Management

### Environment-Specific Configuration

#### Development Configuration
```json
{
  "environment": "DEVELOPMENT",
  "console_output": true,
  "log_file": "logs/dev.log",
  "max_file_size": 10485760,
  "max_files": 5,
  "min_log_level": "DEBUG",
  "database_logging": {
    "enabled": false
  },
  "performance": {
    "queue_size": 10000,
    "batch_size": 50
  }
}
```

#### Production Configuration
```json
{
  "environment": "PRODUCTION",
  "console_output": false,
  "log_file": "logs/prod.log",
  "max_file_size": 104857600,
  "max_files": 20,
  "min_log_level": "INFO",
  "database_logging": {
    "enabled": true,
    "connection_string": "Server=prod-db;Database=ApplicationLogs;Trusted_Connection=True;",
    "table_name": "ApplicationLogs"
  },
  "performance": {
    "queue_size": 100000,
    "batch_size": 500,
    "enable_adaptive_persistence": true
  }
}
```

### Database Configuration

#### SQL Server Setup
```sql
-- Create database
CREATE DATABASE ApplicationLogs;
GO

-- Create log table
USE ApplicationLogs;
GO

CREATE TABLE LogMessages (
    MessageId BIGINT IDENTITY(1,1) PRIMARY KEY,
    InstanceId INT NOT NULL,
    ApplicationName NVARCHAR(255) NOT NULL,
    ProcessName NVARCHAR(255),
    MessageType TINYINT NOT NULL,
    Severity NVARCHAR(20) NOT NULL,
    TimestampSeconds BIGINT NOT NULL,
    TimestampMicroseconds INT NOT NULL,
    ProcessId INT NOT NULL,
    ThreadId INT NOT NULL,
    Component NVARCHAR(255),
    Function NVARCHAR(255),
    FileName NVARCHAR(500),
    LineNumber INT,
    Message NVARCHAR(MAX) NOT NULL,
    CreatedAt DATETIME2 DEFAULT GETUTCDATE(),

    INDEX IX_LogMessages_InstanceId_Timestamp NONCLUSTERED (InstanceId, TimestampSeconds),
    INDEX IX_LogMessages_MessageType NONCLUSTERED (MessageType),
    INDEX IX_LogMessages_Severity NONCLUSTERED (Severity),
    INDEX IX_LogMessages_ApplicationName NONCLUSTERED (ApplicationName)
);
```

#### Connection String Examples
```json
{
  "database_logging": {
    "connection_string": "Server=localhost;Database=ApplicationLogs;Trusted_Connection=True;Connection Timeout=30;",
    "table_name": "LogMessages",
    "connection_pool_size": 10,
    "command_timeout_seconds": 30
  }
}
```

### Shared Memory Configuration

#### Windows FileMaps
```json
{
  "shared_memory": {
    "enabled": true,
    "memory_name": "Global\\ASFMLogger_SharedLogs",
    "size_mb": 100,
    "security_descriptor": "D:(A;;GA;;;WD)"
  }
}
```

#### Linux Shared Memory
```json
{
  "shared_memory": {
    "enabled": true,
    "memory_name": "/asfmlogger_shared_memory",
    "size_mb": 100,
    "permissions": "0666"
  }
}
```

## Multi-Language Integration

### Python Integration

#### Installation
```bash
# Install Python wrapper
pip install -e wrappers/python/

# Or from PyPI (when published)
pip install asfm-logger
```

#### Usage in Python Applications
```python
import asfm_logger

# Configure logger
logger = asfm_logger.get_logger("MyPythonApp")
logger.configure_enhanced(
    enable_database=True,
    database_connection="Server=localhost;Database=Logs;Trusted_Connection=True;",
    log_file="python_app.log"
)

# Use in application
logger.info("Application started", component="Main")
logger.error("Database connection failed", component="Database")
```

### C# Integration

#### Installation
```bash
# Build C# wrapper
cd wrappers/csharp
dotnet build --configuration Release

# Or install from NuGet (when published)
dotnet add package ASFMLogger.Wrapper
```

#### Usage in C# Applications
```csharp
using ASFMLogger.Wrapper;

// Configure logger
var logger = new ASFMLogger("MyCSharpApp");
logger.ConfigureEnhanced(
    enableDatabase: true,
    databaseConnection: "Server=localhost;Database=Logs;Trusted_Connection=True;",
    logFileName: "csharp_app.log"
);

// Use in application
logger.Info("Application started", "Main");
logger.Error("Exception occurred", "ExceptionHandler");
```

### MQL5 Integration (MetaTrader)

#### Installation
1. Copy `ASFMLogger.mq5` to `MetaTrader/MQL5/Experts/ASFMLogger/`
2. Copy `ASFMLogger.mqh` to `MetaTrader/MQL5/Include/ASFMLogger/`
3. Compile in MetaEditor or restart MetaTrader

#### Usage in Expert Advisors
```mql5
#include <ASFMLogger.mq5>

// Initialize in OnInit()
InitializeExpertLogger("MyExpertAdvisor");

// Use in trading logic
LogInfo("Expert Advisor initialized");
LogTradeSignal("BUY", _Symbol, Ask, 0.1);
LogPositionOpen(ticket, _Symbol, OP_BUY, 0.1, Ask);
```

## Monitoring and Health Checks

### Health Check Endpoints

#### REST API Health Check
```bash
curl http://localhost:8080/api/health
```

Expected response:
```json
{
  "status": "healthy",
  "timestamp": "2025-01-01T12:00:00Z",
  "version": "1.0.0",
  "uptime_seconds": 3600,
  "services": {
    "database": "connected",
    "shared_memory": "enabled",
    "web_dashboard": "running"
  },
  "metrics": {
    "total_messages": 150000,
    "error_rate": 0.02,
    "average_response_time_ms": 15
  }
}
```

#### Database Health Check
```sql
-- Check database connectivity and performance
SELECT
    COUNT(*) as total_messages,
    DATEDIFF(MINUTE, MIN(CreatedAt), MAX(CreatedAt)) as uptime_minutes,
    COUNT(*) / DATEDIFF(MINUTE, MIN(CreatedAt), MAX(CreatedAt)) as messages_per_minute
FROM LogMessages;
```

### Performance Monitoring

#### Key Metrics to Monitor
- **Message throughput**: Messages per second
- **Queue sizes**: Current queue utilization
- **Database latency**: Time to persist messages
- **Memory usage**: RAM consumption
- **Error rates**: Percentage of error messages
- **System health**: Overall service health score

#### Monitoring Queries
```sql
-- Daily message volume
SELECT
    CAST(CreatedAt AS DATE) as log_date,
    COUNT(*) as message_count,
    COUNT(CASE WHEN Severity IN ('ERROR', 'CRITICAL') THEN 1 END) as error_count
FROM LogMessages
WHERE CreatedAt >= DATEADD(DAY, -30, GETUTCDATE())
GROUP BY CAST(CreatedAt AS DATE)
ORDER BY log_date DESC;

-- Application performance
SELECT
    ApplicationName,
    COUNT(*) as total_messages,
    AVG(DATEDIFF(MILLISECOND, CreatedAt, CreatedAt)) as avg_processing_time
FROM LogMessages
WHERE CreatedAt >= DATEADD(HOUR, -1, GETUTCDATE())
GROUP BY ApplicationName
ORDER BY total_messages DESC;
```

## Security Considerations

### Access Control
```json
{
  "security": {
    "enable_encryption": false,
    "enable_access_control": true,
    "allowed_applications": [
      "TradingSystem",
      "RiskEngine",
      "ComplianceChecker"
    ],
    "enable_audit_trail": true,
    "audit_retention_days": 90
  }
}
```

### Network Security
- Use TLS/SSL for web dashboard
- Restrict database access to specific IP ranges
- Enable Windows/Linux firewall rules
- Use strong API keys for REST API access

### Data Protection
- Encrypt sensitive configuration data
- Use parameterized queries to prevent SQL injection
- Implement proper input validation
- Regular security audits and updates

## Troubleshooting

### Common Issues

#### High Memory Usage
```bash
# Check queue sizes
curl http://localhost:8080/api/statistics

# Adjust queue configuration
{
  "performance": {
    "queue_size": 50000,
    "enable_adaptive_persistence": true
  }
}
```

#### Database Connection Issues
```bash
# Test database connectivity
curl http://localhost:8080/api/health

# Check database logs
SELECT TOP 100 * FROM LogMessages
WHERE Component = 'Database' AND Severity = 'ERROR'
ORDER BY CreatedAt DESC;
```

#### Performance Issues
```bash
# Check system metrics
curl http://localhost:8080/api/metrics

# Optimize batch sizes
{
  "performance": {
    "batch_size": 500,
    "flush_interval_ms": 10000
  }
}
```

### Debug Mode

#### Enable Debug Logging
```json
{
  "min_log_level": "DEBUG",
  "console_output": true,
  "log_file": "debug.log"
}
```

#### Verbose Diagnostics
```cpp
// Enable verbose logging
auto logger = Logger::getInstance("DebugApp");
logger->configure(true, "debug.log", 10485760, 5, spdlog::level::trace);

// Log detailed information
logger->trace("DebugComponent", "DebugFunction", "Detailed debug information");
```

## Backup and Recovery

### Database Backup
```sql
-- Full database backup
BACKUP DATABASE ApplicationLogs TO DISK = 'C:\backups\ApplicationLogs.bak';

-- Transaction log backup (if in full recovery mode)
BACKUP LOG ApplicationLogs TO DISK = 'C:\backups\ApplicationLogs.trn';
```

### Configuration Backup
```bash
# Backup configuration files
cp /opt/asfmlogger/config/production.json /opt/asfmlogger/backup/config_$(date +%Y%m%d_%H%M%S).json

# Backup log files
tar -czf /opt/asfmlogger/backup/logs_$(date +%Y%m%d_%H%M%S).tar.gz /opt/asfmlogger/logs/
```

### Recovery Procedures

#### Service Recovery
```bash
# Linux/macOS
sudo systemctl reset-failed asfmlogger
sudo systemctl start asfmlogger

# Windows
sc query ASFMLoggerService
net start ASFMLoggerService
```

#### Database Recovery
```sql
-- Restore database
RESTORE DATABASE ApplicationLogs FROM DISK = 'C:\backups\ApplicationLogs.bak'
WITH REPLACE, RECOVERY;
```

## Scaling and High Availability

### Load Balancing
```json
{
  "performance": {
    "enable_load_balancing": true,
    "max_instances_per_application": 5,
    "load_balance_threshold": 80
  }
}
```

### Horizontal Scaling
```bash
# Deploy multiple instances
for i in {1..3}; do
    sudo systemctl start asfmlogger@instance$i
done

# Configure load balancer
{
  "instances": [
    "asfmlogger1:8080",
    "asfmlogger2:8080",
    "asfmlogger3:8080"
  ]
}
```

### Database Clustering
```sql
-- Configure SQL Server clustering
ALTER DATABASE ApplicationLogs SET PARTNER = 'TCP://sql2:5022';

-- Enable read-only routing
ALTER AVAILABILITY GROUP ASFMLoggerAG MODIFY REPLICA ON 'SQL2'
WITH (SECONDARY_ROLE (READ_ONLY_ROUTING_URL = 'TCP://sql2:1433'));
```

## Performance Tuning

### Queue Optimization
```json
{
  "performance": {
    "queue_size": 100000,
    "batch_size": 1000,
    "flush_interval_ms": 5000,
    "enable_compression": true,
    "enable_batching": true
  }
}
```

### Database Optimization
```sql
-- Create indexes for better query performance
CREATE INDEX IX_LogMessages_Timestamp_Type ON LogMessages (TimestampSeconds, MessageType);
CREATE INDEX IX_LogMessages_Application_Timestamp ON LogMessages (ApplicationName, TimestampSeconds);
CREATE INDEX IX_LogMessages_Severity_Timestamp ON LogMessages (Severity, TimestampSeconds);

-- Update statistics
UPDATE STATISTICS LogMessages;
```

### Memory Optimization
```json
{
  "performance": {
    "memory_limit_mb": 500,
    "enable_memory_monitoring": true,
    "memory_warning_threshold": 80,
    "enable_garbage_collection": true
  }
}
```

## API Integration

### REST API Usage
```bash
# Get health status
curl http://localhost:8080/api/health

# Get log statistics
curl http://localhost:8080/api/statistics

# Query logs
curl "http://localhost:8080/api/logs?application=TradingSystem&limit=100"

# Get application metrics
curl http://localhost:8080/api/metrics?hours=24
```

### WebSocket Integration
```javascript
// Connect to WebSocket for real-time updates
const ws = new WebSocket('ws://localhost:8080/ws');

// Subscribe to log stream
ws.send(JSON.stringify({
    type: 'subscribe',
    filters: {
        application: 'TradingSystem',
        level: 'ERROR'
    }
}));

// Receive real-time log messages
ws.onmessage = (event) => {
    const logMessage = JSON.parse(event.data);
    console.log('New log:', logMessage);
};
```

## Maintenance

### Log Rotation
```bash
# Manual log rotation
mv /opt/asfmlogger/logs/application.log /opt/asfmlogger/logs/application_$(date +%Y%m%d_%H%M%S).log
systemctl reload asfmlogger
```

### Database Maintenance
```sql
-- Clean old log entries
DELETE FROM LogMessages
WHERE CreatedAt < DATEADD(DAY, -90, GETUTCDATE());

-- Rebuild indexes
ALTER INDEX ALL ON LogMessages REBUILD;

-- Update statistics
UPDATE STATISTICS LogMessages WITH FULLSCAN;
```

### Service Updates
```bash
# Update service binary
sudo systemctl stop asfmlogger
cp /opt/asfmlogger_new/bin/ASFMLoggerService /opt/asfmlogger/bin/
sudo systemctl start asfmlogger

# Verify service is running
sudo systemctl status asfmlogger
```

## Support and Monitoring

### Health Check Script
```bash
#!/bin/bash
# ASFMLogger health check script

HEALTH_URL="http://localhost:8080/api/health"
LOG_FILE="/var/log/asfmlogger/health_check.log"

# Check service health
if curl -f -s $HEALTH_URL > /dev/null; then
    echo "$(date): ASFMLogger service is healthy" >> $LOG_FILE
else
    echo "$(date): ASFMLogger service is unhealthy" >> $LOG_FILE
    # Send alert
    curl -X POST -H 'Content-type: application/json' \
        --data '{"text":"ASFMLogger service is down"}' \
        $SLACK_WEBHOOK_URL
fi
```

### Performance Monitoring Script
```bash
#!/bin/bash
# ASFMLogger performance monitoring script

API_BASE="http://localhost:8080/api"
LOG_FILE="/var/log/asfmlogger/performance.log"

# Get current statistics
STATS=$(curl -s $API_BASE/statistics)
ERROR_RATE=$(echo $STATS | jq -r '.error_rate // 0')
QUEUE_SIZE=$(echo $STATS | jq -r '.queue_size // 0')
MEMORY_USAGE=$(echo $STATS | jq -r '.memory_usage_mb // 0')

# Check thresholds
if (( $(echo "$ERROR_RATE > 0.05" | bc -l) )); then
    echo "$(date): High error rate detected: $ERROR_RATE" >> $LOG_FILE
fi

if [ "$QUEUE_SIZE" -gt 50000 ]; then
    echo "$(date): Large queue size detected: $QUEUE_SIZE" >> $LOG_FILE
fi

if [ "$MEMORY_USAGE" -gt 80 ]; then
    echo "$(date): High memory usage detected: $MEMORY_USAGE%" >> $LOG_FILE
fi
```

## Uninstallation

### Windows Uninstallation
```batch
# Stop and remove service
net stop ASFMLoggerService
sc delete ASFMLoggerService

# Remove installation directory
rmdir /s /q "C:\ASFMLogger"

# Clean registry (if needed)
reg delete "HKLM\SOFTWARE\ASFMLogger" /f
```

### Linux Uninstallation
```bash
# Stop and disable service
sudo systemctl stop asfmlogger
sudo systemctl disable asfmlogger

# Remove service file
sudo rm /etc/systemd/system/asfmlogger.service

# Remove installation directory
sudo rm -rf /opt/asfmlogger

# Remove user
sudo userdel asfmlogger

# Reload systemd
sudo systemctl daemon-reload
```

## Support

For technical support and questions:
- **GitHub Issues**: https://github.com/AlotfyDev/ASFMLogger/issues
- **Documentation**: https://asfmlogger.dev/docs
- **Email**: support@asfmlogger.dev

## Version History

### v1.0.0 (Current)
- Initial enterprise release
- Multi-language support (C++, Python, C#, MQL5)
- Advanced logging features
- Production deployment support
- Comprehensive documentation

---

*ASFMLogger - Enterprise Logging Made Simple*