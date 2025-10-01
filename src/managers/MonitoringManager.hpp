#ifndef __ASFM_LOGGER_MONITORING_MANAGER_HPP__
#define __ASFM_LOGGER_MONITORING_MANAGER_HPP__

/**
 * ASFMLogger Monitoring and Adaptation Manager
 *
 * Stateful manager class for monitoring and adaptation lifecycle management.
 * Uses MonitoringToolbox internally for all monitoring and adaptation operations.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/MonitoringData.hpp"
#include "toolbox/MonitoringToolbox.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

class MonitoringManager {
private:
    std::string application_name_;
    bool monitoring_enabled_;
    bool adaptation_enabled_;
    mutable std::mutex manager_mutex_;

    // Monitoring configuration
    DWORD metrics_collection_interval_seconds_;
    DWORD health_check_interval_seconds_;
    DWORD adaptation_evaluation_interval_seconds_;
    DWORD history_retention_hours_;

    // Monitoring threads
    std::unique_ptr<std::thread> metrics_collection_thread_;
    std::unique_ptr<std::thread> health_check_thread_;
    std::unique_ptr<std::thread> adaptation_thread_;
    std::atomic<bool> should_run_threads_;

    // Current state
    SystemPerformanceMetrics last_metrics_;
    LoggingSystemHealth last_health_;
    std::chrono::steady_clock::time_point last_adaptation_time_;

    // Alert configuration
    std::vector<MonitoringAlertConfiguration> alert_configs_;
    std::unordered_map<std::string, DWORD> last_alert_times_;

    // Adaptation state
    std::vector<SystemAdaptationRecord> recent_adaptations_;
    std::atomic<size_t> adaptation_count_;

public:
    // =================================================================================
    // CONSTRUCTORS AND DESTRUCTOR
    // =================================================================================

    /**
     * @brief Default constructor
     */
    MonitoringManager();

    /**
     * @brief Constructor with application name
     * @param application_name Name of the application
     */
    explicit MonitoringManager(const std::string& application_name);

    /**
     * @brief Constructor with full configuration
     * @param application_name Name of the application
     * @param collection_interval_seconds Metrics collection interval
     * @param adaptation_interval_seconds Adaptation evaluation interval
     * @param history_retention_hours History retention period
     */
    MonitoringManager(const std::string& application_name,
                     DWORD collection_interval_seconds,
                     DWORD adaptation_interval_seconds,
                     DWORD history_retention_hours);

    /**
     * @brief Copy constructor
     * @param other Manager to copy from
     */
    MonitoringManager(const MonitoringManager& other);

    /**
     * @brief Move constructor
     * @param other Manager to move from
     */
    MonitoringManager(MonitoringManager&& other) noexcept;

    /**
     * @brief Assignment operator
     * @param other Manager to assign from
     * @return Reference to this manager
     */
    MonitoringManager& operator=(const MonitoringManager& other);

    /**
     * @brief Move assignment operator
     * @param other Manager to move from
     * @return Reference to this manager
     */
    MonitoringManager& operator=(MonitoringManager&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~MonitoringManager();

    // =================================================================================
    // APPLICATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Set application name
     * @param application_name Name of the application
     */
    void setApplicationName(const std::string& application_name);

    /**
     * @brief Get application name
     * @return Application name
     */
    std::string getApplicationName() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return application_name_;
    }

    /**
     * @brief Check if manager is configured for an application
     * @return true if application name is set
     */
    bool isConfigured() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return !application_name_.empty();
    }

    // =================================================================================
    // MONITORING CONTROL
    // =================================================================================

    /**
     * @brief Start monitoring system
     * @return true if monitoring started successfully
     */
    bool startMonitoring();

    /**
     * @brief Stop monitoring system
     * @return true if monitoring stopped successfully
     */
    bool stopMonitoring();

    /**
     * @brief Check if monitoring is currently active
     * @return true if monitoring is running
     */
    bool isMonitoringActive() const {
        return monitoring_enabled_ && should_run_threads_;
    }

    /**
     * @brief Enable or disable monitoring
     * @param enabled Whether monitoring should be enabled
     * @return true if state changed successfully
     */
    bool setMonitoringEnabled(bool enabled);

    /**
     * @brief Enable or disable adaptation
     * @param enabled Whether adaptation should be enabled
     * @return true if state changed successfully
     */
    bool setAdaptationEnabled(bool enabled);

    /**
     * @brief Check if adaptation is enabled
     * @return true if adaptation is enabled
     */
    bool isAdaptationEnabled() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return adaptation_enabled_;
    }

    // =================================================================================
    // METRICS COLLECTION
    // =================================================================================

    /**
     * @brief Collect current system performance metrics
     * @return Current system performance metrics
     */
    SystemPerformanceMetrics collectCurrentMetrics();

    /**
     * @brief Collect current logging system health
     * @return Current logging system health
     */
    LoggingSystemHealth collectCurrentHealth();

    /**
     * @brief Get last collected metrics
     * @return Last collected performance metrics
     */
    SystemPerformanceMetrics getLastMetrics() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return last_metrics_;
    }

    /**
     * @brief Get last collected health data
     * @return Last collected health data
     */
    LoggingSystemHealth getLastHealth() const {
        std::lock_guard<std::mutex> lock(manager_mutex_);
        return last_health_;
    }

    /**
     * @brief Get performance history
     * @param time_range_minutes Time range for history
     * @return Vector of performance metrics in the time range
     */
    std::vector<SystemPerformanceMetrics> getPerformanceHistory(DWORD time_range_minutes = 60);

    /**
     * @brief Get health history
     * @param time_range_minutes Time range for history
     * @return Vector of health data in the time range
     */
    std::vector<LoggingSystemHealth> getHealthHistory(DWORD time_range_minutes = 60);

    // =================================================================================
    // ADAPTATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Perform immediate adaptation evaluation
     * @return true if adaptation was performed
     */
    bool evaluateAdaptation();

    /**
     * @brief Get recent adaptation history
     * @param max_entries Maximum number of entries to return
     * @return Vector of recent adaptations
     */
    std::vector<SystemAdaptationRecord> getRecentAdaptations(size_t max_entries = 10);

    /**
     * @brief Get adaptation count
     * @return Total number of adaptations performed
     */
    size_t getAdaptationCount() const {
        return adaptation_count_;
    }

    /**
     * @brief Get time since last adaptation
     * @return Seconds since last adaptation
     */
    DWORD getTimeSinceLastAdaptation() const;

    /**
     * @brief Force system adaptation
     * @param adaptation_type Type of adaptation to force
     * @return true if adaptation was forced successfully
     */
    bool forceAdaptation(const std::string& adaptation_type);

    // =================================================================================
    // TREND ANALYSIS
    // =================================================================================

    /**
     * @brief Analyze performance trends
     * @param time_range_minutes Time range for analysis
     * @return Performance trend analysis results
     */
    PerformanceTrendAnalysis analyzePerformanceTrends(DWORD time_range_minutes = 60);

    /**
     * @brief Analyze health trends
     * @param time_range_minutes Time range for analysis
     * @return Health trend analysis results
     */
    PerformanceTrendAnalysis analyzeHealthTrends(DWORD time_range_minutes = 60);

    /**
     * @brief Detect performance anomalies
     * @return Vector of detected anomalies
     */
    std::vector<std::string> detectPerformanceAnomalies();

    /**
     * @brief Detect health anomalies
     * @return Vector of detected anomalies
     */
    std::vector<std::string> detectHealthAnomalies();

    /**
     * @brief Predict future performance
     * @param prediction_horizon_minutes How far into future to predict
     * @return Predicted performance metrics
     */
    SystemPerformanceMetrics predictFuturePerformance(DWORD prediction_horizon_minutes = 60);

    // =================================================================================
    // ALERT MANAGEMENT
    // =================================================================================

    /**
     * @brief Configure performance alert
     * @param alert_name Name of the alert
     * @param cpu_threshold CPU usage threshold
     * @param memory_threshold Memory usage threshold
     * @param webhook_url Webhook URL for notifications
     * @return true if alert configured successfully
     */
    bool configurePerformanceAlert(const std::string& alert_name,
                                  DWORD cpu_threshold,
                                  DWORD memory_threshold,
                                  const std::string& webhook_url = "");

    /**
     * @brief Configure error rate alert
     * @param alert_name Name of the alert
     * @param error_rate_threshold Error rate threshold
     * @param webhook_url Webhook URL for notifications
     * @return true if alert configured successfully
     */
    bool configureErrorRateAlert(const std::string& alert_name,
                                DWORD error_rate_threshold,
                                const std::string& webhook_url = "");

    /**
     * @brief Remove alert configuration
     * @param alert_name Alert name to remove
     * @return true if alert was found and removed
     */
    bool removeAlertConfiguration(const std::string& alert_name);

    /**
     * @brief Get all alert configurations
     * @return Vector of alert configurations
     */
    std::vector<MonitoringAlertConfiguration> getAlertConfigurations() const;

    /**
     * @brief Check if any alerts should be triggered
     * @return Vector of alerts that should be triggered
     */
    std::vector<std::string> checkAlerts();

    // =================================================================================
    // STATISTICS AND REPORTING
    // =================================================================================

    /**
     * @brief Generate performance report
     * @param time_range_hours Time range for report
     * @return Formatted performance report
     */
    std::string generatePerformanceReport(DWORD time_range_hours = 1);

    /**
     * @brief Generate health report
     * @param time_range_hours Time range for report
     * @return Formatted health report
     */
    std::string generateHealthReport(DWORD time_range_hours = 1);

    /**
     * @brief Generate comprehensive system report
     * @param time_range_hours Time range for report
     * @return Comprehensive system report
     */
    std::string generateComprehensiveReport(DWORD time_range_hours = 1);

    /**
     * @brief Generate trend analysis report
     * @param time_range_hours Time range for analysis
     * @return Trend analysis report
     */
    std::string generateTrendAnalysisReport(DWORD time_range_hours = 24);

    /**
     * @brief Export metrics to CSV format
     * @param time_range_minutes Time range for export
     * @return CSV formatted string
     */
    std::string exportMetricsToCsv(DWORD time_range_minutes = 60);

    /**
     * @brief Export metrics to JSON format
     * @param time_range_minutes Time range for export
     * @return JSON formatted string
     */
    std::string exportMetricsToJson(DWORD time_range_minutes = 60);

    // =================================================================================
    // SYSTEM HEALTH ASSESSMENT
    // =================================================================================

    /**
     * @brief Get current system health score
     * @return Health score (0-100)
     */
    DWORD getCurrentHealthScore();

    /**
     * @brief Get current system stress level
     * @return Stress level (0-100)
     */
    DWORD getCurrentStressLevel();

    /**
     * @brief Check if system is under stress
     * @return true if system is under stress
     */
    bool isUnderStress();

    /**
     * @brief Check if system is healthy
     * @return true if system is healthy
     */
    bool isHealthy();

    /**
     * @brief Get system bottlenecks
     * @return Vector of identified bottlenecks
     */
    std::vector<std::string> getSystemBottlenecks();

    /**
     * @brief Get system recommendations
     * @return Vector of system optimization recommendations
     */
    std::vector<std::string> getSystemRecommendations();

    // =================================================================================
    // PREDICTIVE ANALYSIS
    // =================================================================================

    /**
     * @brief Predict when system will need attention
     * @param prediction_horizon_hours How far ahead to predict
     * @return Prediction results as formatted string
     */
    std::string predictMaintenanceNeeds(DWORD prediction_horizon_hours = 24);

    /**
     * @brief Predict queue overflow timing
     * @return Hours until overflow predicted
     */
    double predictQueueOverflow();

    /**
     * @brief Predict memory exhaustion timing
     * @return Hours until memory exhaustion
     */
    double predictMemoryExhaustion();

    /**
     * @brief Get system capacity utilization
     * @return Capacity utilization percentage (0-100)
     */
    DWORD getCapacityUtilization();

    // =================================================================================
    // CONFIGURATION MANAGEMENT
    // =================================================================================

    /**
     * @brief Configure monitoring intervals
     * @param metrics_collection_seconds Metrics collection interval
     * @param health_check_seconds Health check interval
     * @param adaptation_evaluation_seconds Adaptation evaluation interval
     * @return true if configuration updated successfully
     */
    bool configureMonitoringIntervals(DWORD metrics_collection_seconds,
                                     DWORD health_check_seconds,
                                     DWORD adaptation_evaluation_seconds);

    /**
     * @brief Set history retention period
     * @param retention_hours Hours to retain monitoring history
     * @return true if retention period updated successfully
     */
    bool setHistoryRetention(DWORD retention_hours);

    /**
     * @brief Get current monitoring configuration
     * @return String containing current configuration
     */
    std::string getMonitoringConfiguration() const;

    /**
     * @brief Configure alert system
     * @param webhook_url Webhook URL for alerts
     * @param email_recipients Email recipients for alerts
     * @return true if alert system configured successfully
     */
    bool configureAlertSystem(const std::string& webhook_url = "",
                             const std::string& email_recipients = "");

    // =================================================================================
    // STATISTICS AND METRICS
    // =================================================================================

    /**
     * @brief Get comprehensive statistics
     * @return Comprehensive logging statistics
     */
    ComprehensiveLoggingStatistics getComprehensiveStatistics();

    /**
     * @brief Get performance metrics
     * @return Vector of performance metrics
     */
    std::vector<std::pair<std::string, double>> getPerformanceMetrics();

    /**
     * @brief Get efficiency metrics
     * @return Vector of efficiency metrics
     */
    std::vector<std::pair<std::string, double>> getEfficiencyMetrics();

    /**
     * @brief Get monitoring uptime
     * @return Seconds since monitoring started
     */
    DWORD getMonitoringUptime() const;

    /**
     * @brief Reset all statistics
     */
    void resetStatistics();

    // =================================================================================
    // ADAPTATION STRATEGIES
    // =================================================================================

    /**
     * @brief Get adaptation recommendations
     * @return Vector of adaptation recommendations
     */
    std::vector<std::string> getAdaptationRecommendations();

    /**
     * @brief Apply adaptation strategy
     * @param strategy_name Name of strategy to apply
     * @return true if strategy applied successfully
     */
    bool applyAdaptationStrategy(const std::string& strategy_name);

    /**
     * @brief Get available adaptation strategies
     * @return Vector of available strategy names
     */
    std::vector<std::string> getAvailableAdaptationStrategies();

    /**
     * @brief Create custom adaptation strategy
     * @param strategy_name Name of the strategy
     * @param strategy_description Description of the strategy
     * @return true if strategy created successfully
     */
    bool createCustomAdaptationStrategy(const std::string& strategy_name,
                                       const std::string& strategy_description);

    // =================================================================================
    // MAINTENANCE OPERATIONS
    // =================================================================================

    /**
     * @brief Perform monitoring system maintenance
     * @return true if maintenance completed successfully
     */
    bool performMaintenance();

    /**
     * @brief Clean up old monitoring history
     * @return Number of records cleaned up
     */
    size_t cleanupHistory();

    /**
     * @brief Validate monitoring system integrity
     * @return true if system is in valid state
     */
    bool validateSystemIntegrity();

    /**
     * @brief Get monitoring system status
     * @return Detailed status information
     */
    std::string getSystemStatus();

    // =================================================================================
    // REAL-TIME MONITORING
    // =================================================================================

    /**
     * @brief Get real-time metrics snapshot
     * @return Current system metrics
     */
    SystemPerformanceMetrics getRealTimeMetrics();

    /**
     * @brief Get real-time health snapshot
     * @return Current system health
     */
    LoggingSystemHealth getRealTimeHealth();

    /**
     * @brief Monitor specific metric in real-time
     * @param metric_name Name of metric to monitor
     * @param duration_seconds How long to monitor
     * @return Vector of metric values over time
     */
    std::vector<double> monitorMetricRealTime(const std::string& metric_name, DWORD duration_seconds);

    /**
     * @brief Get current system stress indicators
     * @return Map of stress indicators
     */
    std::unordered_map<std::string, DWORD> getCurrentStressIndicators();

    // =================================================================================
    // UTILITY METHODS
    // =================================================================================

    /**
     * @brief Clone this monitoring manager
     * @return New MonitoringManager with same configuration
     */
    MonitoringManager clone() const;

    /**
     * @brief Swap contents with another manager
     * @param other Manager to swap with
     */
    void swap(MonitoringManager& other) noexcept;

    /**
     * @brief Clear all monitoring data and history
     */
    void clear();

    /**
     * @brief Check if manager is properly initialized
     * @return true if manager has valid configuration
     */
    bool isInitialized() const;

    /**
     * @brief Get manager status as formatted string
     * @return Status information
     */
    std::string getStatus() const;

    /**
     * @brief Export complete monitoring state
     * @return Complete monitoring state as formatted string
     */
    std::string exportCompleteState() const;

private:
    // Private helper methods
    void initializeWithDefaults();
    void metricsCollectionLoop();
    void healthCheckLoop();
    void adaptationLoop();
    void stopAllThreads();
    void updateLastMetrics(const SystemPerformanceMetrics& metrics);
    void updateLastHealth(const LoggingSystemHealth& health);
    void recordAdaptation(const SystemAdaptationRecord& adaptation);
    bool shouldTriggerAdaptation();
    DWORD getCurrentTimestamp() const;
};

// =====================================================================================
// GLOBAL FUNCTIONS
// =====================================================================================

/**
 * @brief Get global monitoring manager for an application
 * @param application_name Name of the application
 * @return Reference to global MonitoringManager
 */
MonitoringManager& GetGlobalMonitoringManager(const std::string& application_name);

/**
 * @brief Create monitoring manager for current application
 * @param application_name Name of the application
 * @return New MonitoringManager for current application
 */
MonitoringManager CreateApplicationMonitoringManager(const std::string& application_name);

/**
 * @brief Start monitoring for application
 * @param application_name Name of the application
 * @return true if monitoring started successfully
 */
bool StartApplicationMonitoring(const std::string& application_name);

/**
 * @brief Stop monitoring for application
 * @param application_name Name of the application
 * @return true if monitoring stopped successfully
 */
bool StopApplicationMonitoring(const std::string& application_name);

#endif // __ASFM_LOGGER_MONITORING_MANAGER_HPP__