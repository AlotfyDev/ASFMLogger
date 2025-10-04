#ifndef __ASFM_LOGGER_MONITORING_TOOLBOX_HPP__
#define __ASFM_LOGGER_MONITORING_TOOLBOX_HPP__

/**
 * ASFMLogger Monitoring and Adaptation Toolbox
 *
 * Static methods for monitoring and adaptation following toolbox architecture.
 * Pure functions for performance monitoring, metrics collection, and adaptive behavior.
 */

#include "structs/LogDataStructures.hpp"
#include "structs/MonitoringData.hpp"
#include "structs/SmartQueueConfiguration.hpp"
#include "structs/DatabaseConfiguration.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

// Forward declarations
struct LogMessageData;

class MonitoringToolbox {
private:
    // Only static variables allowed in toolbox
    static std::deque<SystemPerformanceMetrics> performance_history_;
    static std::deque<LoggingSystemHealth> health_history_;
    static std::vector<SystemAdaptationRecord> adaptation_history_;
    static std::unordered_map<std::string, std::vector<double>> metric_trends_;

public:
    // =================================================================================
    // PERFORMANCE METRICS COLLECTION
    // =================================================================================

    /**
     * @brief Collect current system performance metrics
     * @return Current system performance metrics
     */
    static SystemPerformanceMetrics CollectSystemPerformanceMetrics();

    /**
     * @brief Collect logging system health metrics
     * @return Current logging system health status
     */
    static LoggingSystemHealth CollectLoggingSystemHealth();

    /**
     * @brief Collect comprehensive logging statistics
     * @param application_name Application to collect statistics for
     * @param time_range_hours Time range for statistics collection
     * @return Comprehensive logging statistics
     */
    static ComprehensiveLoggingStatistics CollectComprehensiveStatistics(
        const std::string& application_name,
        DWORD time_range_hours = 1);

    /**
     * @brief Update performance history with new metrics
     * @param metrics New performance metrics to add
     */
    static void UpdatePerformanceHistory(const SystemPerformanceMetrics& metrics);

    /**
     * @brief Update health history with new health data
     * @param health New health data to add
     */
    static void UpdateHealthHistory(const LoggingSystemHealth& health);

    /**
     * @brief Get performance history for trend analysis
     * @param time_range_minutes Time range for history
     * @return Vector of performance metrics in the time range
     */
    static std::vector<SystemPerformanceMetrics> GetPerformanceHistory(DWORD time_range_minutes = 60);

    /**
     * @brief Get health history for trend analysis
     * @param time_range_minutes Time range for history
     * @return Vector of health data in the time range
     */
    static std::vector<LoggingSystemHealth> GetHealthHistory(DWORD time_range_minutes = 60);

    // =================================================================================
    // TREND ANALYSIS
    // =================================================================================

    /**
     * @brief Analyze performance trends over time
     * @param time_range_minutes Time range for analysis
     * @return Performance trend analysis results
     */
    static PerformanceTrendAnalysis AnalyzePerformanceTrends(DWORD time_range_minutes = 60);

    /**
     * @brief Analyze health trends over time
     * @param time_range_minutes Time range for analysis
     * @return Health trend analysis results
     */
    static PerformanceTrendAnalysis AnalyzeHealthTrends(DWORD time_range_minutes = 60);

    /**
     * @brief Detect performance anomalies
     * @param metrics Recent performance metrics
     * @return Vector of detected anomalies
     */
    static std::vector<std::string> DetectPerformanceAnomalies(
        const std::vector<SystemPerformanceMetrics>& metrics);

    /**
     * @brief Detect health anomalies
     * @param health_data Recent health data
     * @return Vector of detected anomalies
     */
    static std::vector<std::string> DetectHealthAnomalies(
        const std::vector<LoggingSystemHealth>& health_data);

    /**
     * @brief Predict future performance based on trends
     * @param prediction_horizon_minutes How far into future to predict
     * @return Predicted performance metrics
     */
    static SystemPerformanceMetrics PredictFuturePerformance(DWORD prediction_horizon_minutes = 60);

    /**
     * @brief Calculate performance trend direction
     * @param metrics Vector of performance metrics
     * @param metric_name Name of metric to analyze
     * @return Trend direction ("UP", "DOWN", "STABLE")
     */
    static std::string CalculateTrendDirection(const std::vector<SystemPerformanceMetrics>& metrics,
                                              const std::string& metric_name);

    // =================================================================================
    // ADAPTIVE BEHAVIOR ENGINE
    // =================================================================================

    /**
     * @brief Evaluate if system adaptation is needed
     * @param current_metrics Current system metrics
     * @param triggers Vector of adaptive triggers to evaluate
     * @return Vector of triggered adaptations
     */
    static std::vector<SystemAdaptationRecord> EvaluateAdaptationNeeds(
        const SystemPerformanceMetrics& current_metrics,
        const std::vector<AdaptiveBehaviorTrigger>& triggers);

    /**
     * @brief Calculate optimal importance threshold for current conditions
     * @param current_metrics Current system metrics
     * @param base_threshold Base importance threshold
     * @return Optimal threshold for current conditions
     */
    static MessageImportance CalculateOptimalImportanceThreshold(
        const SystemPerformanceMetrics& current_metrics,
        MessageImportance base_threshold);

    /**
     * @brief Calculate optimal queue size for current conditions
     * @param current_metrics Current system metrics
     * @param base_queue_size Base queue size
     * @return Optimal queue size for current conditions
     */
    static size_t CalculateOptimalQueueSize(const SystemPerformanceMetrics& current_metrics,
                                           size_t base_queue_size);

    /**
     * @brief Calculate optimal batch size for current conditions
     * @param current_metrics Current system metrics
     * @param base_batch_size Base batch size
     * @return Optimal batch size for current conditions
     */
    static size_t CalculateOptimalBatchSize(const SystemPerformanceMetrics& current_metrics,
                                          size_t base_batch_size);

    /**
     * @brief Determine if system is under stress
     * @param current_metrics Current system metrics
     * @param stress_thresholds Stress threshold configuration
     * @return Stress level (0-100)
     */
    static DWORD CalculateSystemStressLevel(const SystemPerformanceMetrics& current_metrics,
                                           const std::unordered_map<std::string, DWORD>& stress_thresholds);

    /**
     * @brief Generate adaptation recommendations
     * @param current_metrics Current system metrics
     * @param current_health Current system health
     * @return Vector of adaptation recommendations
     */
    static std::vector<std::string> GenerateAdaptationRecommendations(
        const SystemPerformanceMetrics& current_metrics,
        const LoggingSystemHealth& current_health);

    // =================================================================================
    // ALERT MANAGEMENT
    // =================================================================================

    /**
     * @brief Check if alert should be triggered
     * @param current_metrics Current system metrics
     * @param alert_config Alert configuration
     * @return true if alert should be triggered
     */
    static bool ShouldTriggerAlert(const SystemPerformanceMetrics& current_metrics,
                                  const MonitoringAlertConfiguration& alert_config);

    /**
     * @brief Generate alert message
     * @param alert_config Alert configuration
     * @param current_metrics Current system metrics
     * @param trigger_reason Reason for alert
     * @return Formatted alert message
     */
    static std::string GenerateAlertMessage(const MonitoringAlertConfiguration& alert_config,
                                           const SystemPerformanceMetrics& current_metrics,
                                           const std::string& trigger_reason);

    /**
     * @brief Send alert via webhook
     * @param webhook_url Webhook URL
     * @param alert_message Alert message to send
     * @return true if alert sent successfully
     */
    static bool SendWebhookAlert(const std::string& webhook_url, const std::string& alert_message);

    /**
     * @brief Send alert via email
     * @param recipients Email recipients (comma-separated)
     * @param subject Email subject
     * @param message Email message
     * @return true if email sent successfully
     */
    static bool SendEmailAlert(const std::string& recipients,
                              const std::string& subject,
                              const std::string& message);

    /**
     * @brief Update alert cooldown status
     * @param alert_config Alert configuration to update
     * @param current_time Current timestamp
     * @return true if alert is ready to fire again
     */
    static bool UpdateAlertCooldown(MonitoringAlertConfiguration& alert_config, DWORD current_time);

    // =================================================================================
    // STATISTICS AND REPORTING
    // =================================================================================

    /**
     * @brief Generate performance report
     * @param time_range_hours Time range for report
     * @param application_name Application name (optional)
     * @return Formatted performance report
     */
    static std::string GeneratePerformanceReport(DWORD time_range_hours = 1,
                                                const std::string& application_name = "");

    /**
     * @brief Generate health report
     * @param time_range_hours Time range for report
     * @return Formatted health report
     */
    static std::string GenerateHealthReport(DWORD time_range_hours = 1);

    /**
     * @brief Generate comprehensive system report
     * @param time_range_hours Time range for report
     * @param application_name Application name (optional)
     * @return Comprehensive system report
     */
    static std::string GenerateComprehensiveReport(DWORD time_range_hours = 1,
                                                  const std::string& application_name = "");

    /**
     * @brief Generate trend analysis report
     * @param time_range_hours Time range for analysis
     * @return Trend analysis report
     */
    static std::string GenerateTrendAnalysisReport(DWORD time_range_hours = 24);

    /**
     * @brief Export metrics to CSV format
     * @param metrics Vector of performance metrics
     * @return CSV formatted string
     */
    static std::string ExportMetricsToCsv(const std::vector<SystemPerformanceMetrics>& metrics);

    /**
     * @brief Export metrics to JSON format
     * @param metrics Vector of performance metrics
     * @return JSON formatted string
     */
    static std::string ExportMetricsToJson(const std::vector<SystemPerformanceMetrics>& metrics);

    // =================================================================================
    // SYSTEM HEALTH ASSESSMENT
    // =================================================================================

    /**
     * @brief Assess overall system health
     * @param current_metrics Current system metrics
     * @param current_health Current health data
     * @return Health score (0-100)
     */
    static DWORD AssessSystemHealth(const SystemPerformanceMetrics& current_metrics,
                                   const LoggingSystemHealth& current_health);

    /**
     * @brief Assess logging system health
     * @param health_data Recent health data
     * @return Logging system health score (0-100)
     */
    static DWORD AssessLoggingSystemHealth(const std::vector<LoggingSystemHealth>& health_data);

    /**
     * @brief Assess database health
     * @param connection_count Current database connections
     * @param error_count Recent database errors
     * @param response_time_ms Average response time
     * @return Database health score (0-100)
     */
    static DWORD AssessDatabaseHealth(size_t connection_count,
                                     size_t error_count,
                                     DWORD response_time_ms);

    /**
     * @brief Assess queue health
     * @param total_queues Total number of queues
     * @param overflow_events Recent overflow events
     * @param average_latency_ms Average queue latency
     * @return Queue health score (0-100)
     */
    static DWORD AssessQueueHealth(size_t total_queues,
                                  size_t overflow_events,
                                  DWORD average_latency_ms);

    /**
     * @brief Identify system bottlenecks
     * @param current_metrics Current system metrics
     * @return Vector of identified bottlenecks
     */
    static std::vector<std::string> IdentifyBottlenecks(const SystemPerformanceMetrics& current_metrics);

    // =================================================================================
    // PREDICTIVE ANALYSIS
    // =================================================================================

    /**
     * @brief Predict when system will need attention
     * @param current_metrics Current system metrics
     * @param prediction_horizon_hours How far ahead to predict
     * @return Prediction results as formatted string
     */
    static std::string PredictMaintenanceNeeds(const SystemPerformanceMetrics& current_metrics,
                                              DWORD prediction_horizon_hours = 24);

    /**
     * @brief Predict queue overflow timing
     * @param current_queue_size Current queue size
     * @param message_rate Current message rate
     * @param max_queue_size Maximum queue size
     * @return Hours until overflow predicted
     */
    static double PredictQueueOverflow(DWORD current_queue_size,
                                      DWORD message_rate,
                                      DWORD max_queue_size);

    /**
     * @brief Predict memory exhaustion timing
     * @param current_memory_usage Current memory usage
     * @param memory_increase_rate Rate of memory increase per hour
     * @param max_memory Maximum available memory
     * @return Hours until memory exhaustion
     */
    static double PredictMemoryExhaustion(DWORD current_memory_usage,
                                         double memory_increase_rate,
                                         DWORD max_memory);

    /**
     * @brief Calculate system capacity utilization
     * @param current_metrics Current system metrics
     * @return Capacity utilization percentage (0-100)
     */
    static DWORD CalculateCapacityUtilization(const SystemPerformanceMetrics& current_metrics);

    // =================================================================================
    // ADAPTATION STRATEGIES
    // =================================================================================

    /**
     * @brief Generate CPU optimization strategy
     * @param current_cpu_usage Current CPU usage
     * @param target_cpu_usage Target CPU usage
     * @return Vector of optimization strategies
     */
    static std::vector<std::string> GenerateCpuOptimizationStrategy(DWORD current_cpu_usage,
                                                                  DWORD target_cpu_usage);

    /**
     * @brief Generate memory optimization strategy
     * @param current_memory_usage Current memory usage
     * @param target_memory_usage Target memory usage
     * @return Vector of optimization strategies
     */
    static std::vector<std::string> GenerateMemoryOptimizationStrategy(DWORD current_memory_usage,
                                                                     DWORD target_memory_usage);

    /**
     * @brief Generate queue optimization strategy
     * @param current_queue_stats Current queue statistics
     * @param target_performance Target performance metrics
     * @return Vector of optimization strategies
     */
    static std::vector<std::string> GenerateQueueOptimizationStrategy(
        const std::vector<SmartQueueStatistics>& current_queue_stats,
        const std::unordered_map<std::string, DWORD>& target_performance);

    /**
     * @brief Generate database optimization strategy
     * @param current_db_metrics Current database metrics
     * @param target_performance Target performance metrics
     * @return Vector of optimization strategies
     */
    static std::vector<std::string> GenerateDatabaseOptimizationStrategy(
        const std::vector<DatabaseOperationResult>& current_db_metrics,
        const std::unordered_map<std::string, DWORD>& target_performance);

    // =================================================================================
    // METRICS CALCULATION
    // =================================================================================

    /**
     * @brief Calculate message throughput (messages per second)
     * @param message_count Number of messages
     * @param time_period_seconds Time period in seconds
     * @return Messages per second
     */
    static double CalculateMessageThroughput(uint64_t message_count, DWORD time_period_seconds);

    /**
     * @brief Calculate error rate (errors per second)
     * @param error_count Number of errors
     * @param time_period_seconds Time period in seconds
     * @return Errors per second
     */
    static double CalculateErrorRate(uint64_t error_count, DWORD time_period_seconds);

    /**
     * @brief Calculate average response time
     * @param response_times Vector of response times in milliseconds
     * @return Average response time in milliseconds
     */
    static DWORD CalculateAverageResponseTime(const std::vector<DWORD>& response_times);

    /**
     * @brief Calculate performance percentiles
     * @param values Vector of performance values
     * @return Map of percentiles (50th, 90th, 95th, 99th)
     */
    static std::unordered_map<std::string, DWORD> CalculatePerformancePercentiles(
        const std::vector<DWORD>& values);

    /**
     * @brief Calculate system efficiency score
     * @param metrics System metrics to evaluate
     * @return Efficiency score (0-100)
     */
    static DWORD CalculateSystemEfficiencyScore(const SystemPerformanceMetrics& metrics);

    // =================================================================================
    // CONFIGURATION AND SETUP
    // =================================================================================

    /**
     * @brief Initialize monitoring system
     * @param collection_interval_seconds How often to collect metrics
     * @param history_retention_hours How long to retain history
     * @return true if initialization successful
     */
    static bool InitializeMonitoringSystem(DWORD collection_interval_seconds = 10,
                                          DWORD history_retention_hours = 24);

    /**
     * @brief Configure performance monitoring thresholds
     * @param cpu_warning_threshold CPU warning threshold
     * @param memory_warning_threshold Memory warning threshold
     * @param error_rate_warning_threshold Error rate warning threshold
     * @return true if thresholds configured successfully
     */
    static bool ConfigurePerformanceThresholds(DWORD cpu_warning_threshold = 70,
                                              DWORD memory_warning_threshold = 80,
                                              DWORD error_rate_warning_threshold = 10);

    /**
     * @brief Configure alert system
     * @param webhook_url Webhook URL for alerts
     * @param email_recipients Email recipients for alerts
     * @return true if alert system configured successfully
     */
    static bool ConfigureAlertSystem(const std::string& webhook_url = "",
                                    const std::string& email_recipients = "");

    /**
     * @brief Start monitoring collection
     * @return true if monitoring started successfully
     */
    static bool StartMonitoring();

    /**
     * @brief Stop monitoring collection
     * @return true if monitoring stopped successfully
     */
    static bool StopMonitoring();

    // =================================================================================
    // UTILITY FUNCTIONS
    // =================================================================================

    /**
     * @brief Convert performance metrics to string
     * @param metrics Performance metrics to format
     * @return Human-readable metrics description
     */
    static std::string PerformanceMetricsToString(const SystemPerformanceMetrics& metrics);

    /**
     * @brief Convert health data to string
     * @param health Health data to format
     * @return Human-readable health description
     */
    static std::string HealthDataToString(const LoggingSystemHealth& health);

    /**
     * @brief Convert statistics to string
     * @param stats Statistics to format
     * @return Human-readable statistics description
     */
    static std::string StatisticsToString(const ComprehensiveLoggingStatistics& stats);

    /**
     * @brief Convert trend analysis to string
     * @param analysis Trend analysis to format
     * @return Human-readable trend description
     */
    static std::string TrendAnalysisToString(const PerformanceTrendAnalysis& analysis);

    /**
     * @brief Get current timestamp for monitoring operations
     * @return Current timestamp
     */
    static DWORD GetCurrentTimestamp();

    /**
     * @brief Format duration for display
     * @param seconds Duration in seconds
     * @return Formatted duration string
     */
    static std::string FormatDuration(DWORD seconds);

    /**
     * @brief Format bytes for display
     * @param bytes Bytes to format
     * @return Formatted size string
     */
    static std::string FormatBytes(size_t bytes);

private:
    // Private helper methods
    static uint32_t GenerateCollectionId();
    static uint32_t GenerateHealthCheckId();
    static uint32_t GenerateStatisticsId();
    static uint32_t GenerateAnalysisId();
    static uint32_t GenerateAdaptationId();
    static std::string FormatTimestamp(DWORD timestamp);
    static void CollectWindowsPerformanceMetrics(SystemPerformanceMetrics& metrics);
    static void CollectLoggingSystemMetrics(LoggingSystemHealth& health);
    static void CollectApplicationMetrics(const std::string& application_name,
                                        ComprehensiveLoggingStatistics& stats);
    static double CalculateTrendSlope(const std::vector<double>& values);
    static std::vector<double> ExtractMetricValues(const std::vector<SystemPerformanceMetrics>& metrics,
                                                   const std::string& metric_name);
    static bool IsAnomalousValue(double value, double mean, double standard_deviation, double threshold);
    static void InitializeDefaultThresholds();
    static bool IsInitialized();
};

#endif // __ASFM_LOGGER_MONITORING_TOOLBOX_HPP__