/**
 * ASFMLogger Monitoring and Adaptation Toolbox Implementation
 *
 * Static methods for monitoring and adaptation following toolbox architecture.
 * Pure functions for performance monitoring, metrics collection, and adaptive behavior.
 */

#include "MonitoringToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include "../structs/LogDataStructures.hpp"
#include "../structs/MonitoringData.hpp"
#include "../structs/SmartQueueConfiguration.hpp"
#include "../structs/DatabaseConfiguration.hpp"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <Windows.h>
#include <ctime>

// Static variables
std::deque<SystemPerformanceMetrics> MonitoringToolbox::performance_history_;
std::deque<LoggingSystemHealth> MonitoringToolbox::health_history_;
std::vector<SystemAdaptationRecord> MonitoringToolbox::adaptation_history_;
std::unordered_map<std::string, std::vector<double>> MonitoringToolbox::metric_trends_;

// =================================================================================
// PERFORMANCE METRICS COLLECTION
// =================================================================================

SystemPerformanceMetrics MonitoringToolbox::CollectSystemPerformanceMetrics() {
    SystemPerformanceMetrics metrics;
    memset(&metrics, 0, sizeof(SystemPerformanceMetrics));

    metrics.collection_id = GenerateCollectionId();
    metrics.collection_time = GetCurrentTimestamp();

    // Collect Windows performance metrics
    CollectWindowsPerformanceMetrics(metrics);

    return metrics;
}

LoggingSystemHealth MonitoringToolbox::CollectLoggingSystemHealth() {
    LoggingSystemHealth health;
    memset(&health, 0, sizeof(LoggingSystemHealth));

    health.health_check_id = GenerateHealthCheckId();
    health.check_time = GetCurrentTimestamp();

    // Collect logging system specific metrics
    CollectLoggingSystemMetrics(health);

    return health;
}

ComprehensiveLoggingStatistics MonitoringToolbox::CollectComprehensiveStatistics(
    const std::string& application_name,
    DWORD time_range_hours) {
    ComprehensiveLoggingStatistics stats;
    memset(&stats, 0, sizeof(ComprehensiveLoggingStatistics));

    stats.statistics_id = GenerateStatisticsId();
    strcpy_s(stats.application_name, application_name.c_str());
    stats.collection_period_seconds = time_range_hours * 3600;

    // Collect application-specific metrics
    CollectApplicationMetrics(application_name, stats);

    return stats;
}

void MonitoringToolbox::UpdatePerformanceHistory(const SystemPerformanceMetrics& metrics) {
    performance_history_.push_back(metrics);

    // Keep only last 24 hours of data (assuming 10-second intervals)
    while (performance_history_.size() > 8640) {
        performance_history_.pop_front();
    }
}

void MonitoringToolbox::UpdateHealthHistory(const LoggingSystemHealth& health) {
    health_history_.push_back(health);

    // Keep only last 24 hours of data
    while (health_history_.size() > 8640) {
        health_history_.pop_front();
    }
}

std::vector<SystemPerformanceMetrics> MonitoringToolbox::GetPerformanceHistory(DWORD time_range_minutes) {
    std::vector<SystemPerformanceMetrics> history;
    DWORD current_time = GetCurrentTimestamp();
    DWORD cutoff_time = current_time - (time_range_minutes * 60);

    for (const auto& metrics : performance_history_) {
        if (metrics.collection_time >= cutoff_time) {
            history.push_back(metrics);
        }
    }

    return history;
}

std::vector<LoggingSystemHealth> MonitoringToolbox::GetHealthHistory(DWORD time_range_minutes) {
    std::vector<LoggingSystemHealth> history;
    DWORD current_time = GetCurrentTimestamp();
    DWORD cutoff_time = current_time - (time_range_minutes * 60);

    for (const auto& health : health_history_) {
        if (health.check_time >= cutoff_time) {
            history.push_back(health);
        }
    }

    return history;
}

// =================================================================================
// TREND ANALYSIS
// =================================================================================

PerformanceTrendAnalysis MonitoringToolbox::AnalyzePerformanceTrends(DWORD time_range_minutes) {
    PerformanceTrendAnalysis analysis;
    memset(&analysis, 0, sizeof(PerformanceTrendAnalysis));

    analysis.analysis_id = GenerateAnalysisId();
    analysis.analysis_start_time = GetCurrentTimestamp() - (time_range_minutes * 60);
    analysis.analysis_end_time = GetCurrentTimestamp();

    std::vector<SystemPerformanceMetrics> history = GetPerformanceHistory(time_range_minutes);

    if (history.empty()) {
        strcpy_s(analysis.analysis_period, "NO_DATA");
        return analysis;
    }

    // Analyze CPU trends
    std::vector<double> cpu_values;
    for (const auto& metrics : history) {
        cpu_values.push_back(static_cast<double>(metrics.cpu_usage_percent));
    }

    double cpu_trend = CalculateTrendSlope(cpu_values);
    analysis.cpu_usage_trending_up = cpu_trend > 0.1;
    analysis.cpu_usage_change_rate = cpu_trend;

    // Analyze memory trends
    std::vector<double> memory_values;
    for (const auto& metrics : history) {
        memory_values.push_back(static_cast<double>(metrics.memory_usage_percent));
    }

    double memory_trend = CalculateTrendSlope(memory_values);
    analysis.memory_usage_trending_up = memory_trend > 0.1;
    analysis.memory_usage_change_rate = memory_trend;

    // Set analysis period
    if (time_range_minutes >= 1440) {
        strcpy_s(analysis.analysis_period, "24H");
    } else if (time_range_minutes >= 60) {
        strcpy_s(analysis.analysis_period, "1H");
    } else {
        strcpy_s(analysis.analysis_period, "1M");
    }

    return analysis;
}

PerformanceTrendAnalysis MonitoringToolbox::AnalyzeHealthTrends(DWORD time_range_minutes) {
    PerformanceTrendAnalysis analysis;
    memset(&analysis, 0, sizeof(PerformanceTrendAnalysis));

    analysis.analysis_id = GenerateAnalysisId();
    analysis.analysis_start_time = GetCurrentTimestamp() - (time_range_minutes * 60);
    analysis.analysis_end_time = GetCurrentTimestamp();

    std::vector<LoggingSystemHealth> history = GetHealthHistory(time_range_minutes);

    if (history.empty()) {
        strcpy_s(analysis.analysis_period, "NO_DATA");
        return analysis;
    }

    // Analyze error rate trends
    std::vector<double> error_rate_values;
    for (const auto& health : history) {
        error_rate_values.push_back(static_cast<double>(health.error_rate_per_second));
    }

    double error_trend = CalculateTrendSlope(error_rate_values);
    analysis.error_rate_trending_up = error_trend > 0.1;
    analysis.error_rate_change_rate = error_trend;

    // Analyze throughput trends
    std::vector<double> throughput_values;
    for (const auto& health : history) {
        throughput_values.push_back(static_cast<double>(health.message_throughput_per_second));
    }

    double throughput_trend = CalculateTrendSlope(throughput_values);
    analysis.message_rate_trending_up = throughput_trend > 0.1;
    analysis.message_rate_change_rate = throughput_trend;

    return analysis;
}

std::vector<std::string> MonitoringToolbox::DetectPerformanceAnomalies(
    const std::vector<SystemPerformanceMetrics>& metrics) {
    std::vector<std::string> anomalies;

    if (metrics.size() < 10) {
        return anomalies; // Need sufficient data for anomaly detection
    }

    // Calculate baseline (median of recent values)
    std::vector<double> cpu_values;
    for (const auto& metric : metrics) {
        cpu_values.push_back(static_cast<double>(metric.cpu_usage_percent));
    }

    std::sort(cpu_values.begin(), cpu_values.end());
    double cpu_baseline = cpu_values[cpu_values.size() / 2];

    // Check for anomalies
    size_t start_index = (metrics.size() > 10) ? metrics.size() - 10 : 0;
    for (size_t i = start_index; i < metrics.size(); ++i) {
        double cpu_usage = static_cast<double>(metrics[i].cpu_usage_percent);

        if (std::abs(cpu_usage - cpu_baseline) > 50.0) { // 50% deviation threshold
            anomalies.push_back("CPU usage anomaly detected");
            break;
        }
    }

    return anomalies;
}

std::vector<std::string> MonitoringToolbox::DetectHealthAnomalies(
    const std::vector<LoggingSystemHealth>& health_data) {
    std::vector<std::string> anomalies;

    if (health_data.size() < 5) {
        return anomalies;
    }

    // Check for sudden spikes in error rate
    for (size_t i = 1; i < health_data.size(); ++i) {
        double current_error_rate = static_cast<double>(health_data[i].error_rate_per_second);
        double previous_error_rate = static_cast<double>(health_data[i-1].error_rate_per_second);

        if (current_error_rate > previous_error_rate * 5.0) { // 5x increase
            anomalies.push_back("Sudden error rate spike detected");
        }
    }

    return anomalies;
}

SystemPerformanceMetrics MonitoringToolbox::PredictFuturePerformance(DWORD prediction_horizon_minutes) {
    SystemPerformanceMetrics prediction;
    memset(&prediction, 0, sizeof(SystemPerformanceMetrics));

    std::vector<SystemPerformanceMetrics> recent_history = GetPerformanceHistory(60); // Last hour

    if (recent_history.empty()) {
        return prediction;
    }

    // Simple linear extrapolation
    std::vector<double> cpu_values;
    for (const auto& metrics : recent_history) {
        cpu_values.push_back(static_cast<double>(metrics.cpu_usage_percent));
    }

    double trend = CalculateTrendSlope(cpu_values);
    double latest_cpu = cpu_values.back();
    double predicted_cpu = latest_cpu + (trend * prediction_horizon_minutes);

    // Clamp to reasonable bounds
    if (predicted_cpu < 0) predicted_cpu = 0;
    if (predicted_cpu > 100) predicted_cpu = 100;

    prediction.cpu_usage_percent = static_cast<DWORD>(predicted_cpu);
    prediction.collection_time = GetCurrentTimestamp() + (prediction_horizon_minutes * 60);

    return prediction;
}

std::string MonitoringToolbox::CalculateTrendDirection(const std::vector<SystemPerformanceMetrics>& metrics,
                                                      const std::string& metric_name) {
    if (metrics.size() < 2) {
        return "INSUFFICIENT_DATA";
    }

    std::vector<double> values;
    for (const auto& metric : metrics) {
        if (metric_name == "cpu") {
            values.push_back(static_cast<double>(metric.cpu_usage_percent));
        } else if (metric_name == "memory") {
            values.push_back(static_cast<double>(metric.memory_usage_percent));
        }
    }

    if (values.size() < 2) {
        return "INSUFFICIENT_DATA";
    }

    double trend = CalculateTrendSlope(values);

    if (std::abs(trend) < 0.1) {
        return "STABLE";
    } else if (trend > 0) {
        return "UP";
    } else {
        return "DOWN";
    }
}

// =================================================================================
// ADAPTIVE BEHAVIOR ENGINE
// =================================================================================

std::vector<SystemAdaptationRecord> MonitoringToolbox::EvaluateAdaptationNeeds(
    const SystemPerformanceMetrics& current_metrics,
    const std::vector<AdaptiveBehaviorTrigger>& triggers) {
    std::vector<SystemAdaptationRecord> triggered_adaptations;

    for (const auto& trigger : triggers) {
        if (!trigger.is_enabled) {
            continue;
        }

        bool should_trigger = false;

        // Evaluate trigger conditions
        if (current_metrics.cpu_usage_percent > trigger.cpu_threshold_percent) {
            should_trigger = true;
        } else if (current_metrics.memory_usage_percent > trigger.memory_threshold_percent) {
            should_trigger = true;
        }

        if (should_trigger) {
            SystemAdaptationRecord adaptation;
            memset(&adaptation, 0, sizeof(SystemAdaptationRecord));

            adaptation.adaptation_id = GenerateAdaptationId();
            strcpy_s(adaptation.adaptation_type, trigger.adaptation_action);
            strcpy_s(adaptation.trigger_reason, trigger.trigger_name);

            adaptation.cpu_usage_before = current_metrics.cpu_usage_percent;
            adaptation.memory_usage_before = current_metrics.memory_usage_percent;
            adaptation.adaptation_start_time = GetCurrentTimestamp();

            triggered_adaptations.push_back(adaptation);
        }
    }

    return triggered_adaptations;
}

MessageImportance MonitoringToolbox::CalculateOptimalImportanceThreshold(
    const SystemPerformanceMetrics& current_metrics,
    MessageImportance base_threshold) {
    DWORD stress_level = CalculateSystemStressLevel(current_metrics, {});

    if (stress_level > 80) {
        // Under high stress - increase threshold to reduce logging
        return static_cast<MessageImportance>((static_cast<int>(base_threshold) + 1 < static_cast<int>(MessageImportance::CRITICAL)) ? static_cast<int>(base_threshold) + 1 : static_cast<int>(MessageImportance::CRITICAL));
    } else if (stress_level < 30) {
        // Low stress - can be more verbose
        return static_cast<MessageImportance>((static_cast<int>(base_threshold) - 1 > static_cast<int>(MessageImportance::LOW)) ? static_cast<int>(base_threshold) - 1 : static_cast<int>(MessageImportance::LOW));
    }

    return base_threshold;
}

size_t MonitoringToolbox::CalculateOptimalQueueSize(const SystemPerformanceMetrics& current_metrics,
                                                   size_t base_queue_size) {
    DWORD memory_usage = current_metrics.memory_usage_percent;

    if (memory_usage > 80) {
        return base_queue_size / 2; // Reduce queue size under memory pressure
    } else if (memory_usage < 50) {
        return base_queue_size * 2; // Increase queue size when memory is available
    }

    return base_queue_size;
}

size_t MonitoringToolbox::CalculateOptimalBatchSize(const SystemPerformanceMetrics& current_metrics,
                                                   size_t base_batch_size) {
    DWORD cpu_usage = current_metrics.cpu_usage_percent;

    if (cpu_usage > 70) {
        return base_batch_size / 2; // Smaller batches under CPU pressure
    } else if (cpu_usage < 30) {
        return base_batch_size * 2; // Larger batches when CPU is available
    }

    return base_batch_size;
}

DWORD MonitoringToolbox::CalculateSystemStressLevel(const SystemPerformanceMetrics& current_metrics,
                                                   const std::unordered_map<std::string, DWORD>& stress_thresholds) {
    DWORD cpu_stress = (current_metrics.cpu_usage_percent < DWORD(100)) ? current_metrics.cpu_usage_percent : DWORD(100);
    DWORD memory_stress = (current_metrics.memory_usage_percent < DWORD(100)) ? current_metrics.memory_usage_percent : DWORD(100);

    // Weighted average of stress indicators
    DWORD overall_stress = (cpu_stress * 0.6) + (memory_stress * 0.4);

    return (overall_stress < DWORD(100)) ? overall_stress : DWORD(100);
}

std::vector<std::string> MonitoringToolbox::GenerateAdaptationRecommendations(
    const SystemPerformanceMetrics& current_metrics,
    const LoggingSystemHealth& current_health) {
    std::vector<std::string> recommendations;

    if (current_metrics.cpu_usage_percent > 80) {
        recommendations.push_back("Consider reducing logging frequency due to high CPU usage");
    }

    if (current_metrics.memory_usage_percent > 85) {
        recommendations.push_back("Consider reducing queue sizes due to high memory usage");
    }

    if (current_health.error_rate_per_second > 10) {
        recommendations.push_back("High error rate detected - consider increasing retry delays");
    }

    if (current_health.queue_overflow_events > 0) {
        recommendations.push_back("Queue overflows detected - consider increasing queue capacity");
    }

    return recommendations;
}

// =================================================================================
// ALERT MANAGEMENT
// =================================================================================

bool MonitoringToolbox::ShouldTriggerAlert(const SystemPerformanceMetrics& current_metrics,
                                          const MonitoringAlertConfiguration& alert_config) {
    if (!alert_config.alert_enabled) {
        return false;
    }

    DWORD current_time = GetCurrentTimestamp();
    DWORD time_since_last_alert = current_time - alert_config.last_alert_time;

    if (time_since_last_alert < alert_config.alert_cooldown_seconds) {
        return false; // Still in cooldown period
    }

    // Check if any threshold is exceeded
    if (current_metrics.cpu_usage_percent > alert_config.cpu_threshold_percent) {
        return true;
    }

    if (current_metrics.memory_usage_percent > alert_config.memory_threshold_percent) {
        return true;
    }

    return false;
}

std::string MonitoringToolbox::GenerateAlertMessage(const MonitoringAlertConfiguration& alert_config,
                                                   const SystemPerformanceMetrics& current_metrics,
                                                   const std::string& trigger_reason) {
    std::ostringstream oss;

    oss << "ALERT: " << alert_config.alert_name << std::endl;
    oss << "Reason: " << trigger_reason << std::endl;
    oss << "CPU Usage: " << current_metrics.cpu_usage_percent << "%" << std::endl;
    oss << "Memory Usage: " << current_metrics.memory_usage_percent << "%" << std::endl;
    oss << "Timestamp: " << FormatTimestamp(current_metrics.collection_time) << std::endl;

    return oss.str();
}

bool MonitoringToolbox::SendWebhookAlert(const std::string& webhook_url, const std::string& alert_message) {
    // In a real implementation, this would send HTTP POST to webhook
    // For now, simulate successful sending
    return true;
}

bool MonitoringToolbox::SendEmailAlert(const std::string& recipients,
                                      const std::string& subject,
                                      const std::string& message) {
    // In a real implementation, this would send email
    // For now, simulate successful sending
    return true;
}

bool MonitoringToolbox::UpdateAlertCooldown(MonitoringAlertConfiguration& alert_config, DWORD current_time) {
    DWORD time_since_last_alert = current_time - alert_config.last_alert_time;

    if (time_since_last_alert >= alert_config.alert_cooldown_seconds) {
        alert_config.last_alert_time = current_time;
        alert_config.alert_count++;
        return true; // Alert can be sent
    }

    return false; // Still in cooldown
}

// =================================================================================
// STATISTICS AND REPORTING
// =================================================================================

std::string MonitoringToolbox::GeneratePerformanceReport(DWORD time_range_hours,
                                                        const std::string& application_name) {
    std::ostringstream oss;

    oss << "=== PERFORMANCE REPORT ===" << std::endl;
    oss << "Application: " << application_name << std::endl;
    oss << "Time Range: " << time_range_hours << " hours" << std::endl;
    oss << "Generated: " << FormatTimestamp(GetCurrentTimestamp()) << std::endl;
    oss << std::endl;

    std::vector<SystemPerformanceMetrics> history = GetPerformanceHistory(time_range_hours * 60);

    if (history.empty()) {
        oss << "No performance data available for the specified time range." << std::endl;
        return oss.str();
    }

    // Calculate averages
    double avg_cpu = 0, avg_memory = 0;
    for (const auto& metrics : history) {
        avg_cpu += metrics.cpu_usage_percent;
        avg_memory += metrics.memory_usage_percent;
    }
    avg_cpu /= history.size();
    avg_memory /= history.size();

    oss << "Average CPU Usage: " << std::fixed << std::setprecision(1) << avg_cpu << "%" << std::endl;
    oss << "Average Memory Usage: " << std::fixed << std::setprecision(1) << avg_memory << "%" << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::GenerateHealthReport(DWORD time_range_hours) {
    std::ostringstream oss;

    oss << "=== HEALTH REPORT ===" << std::endl;
    oss << "Time Range: " << time_range_hours << " hours" << std::endl;
    oss << "Generated: " << FormatTimestamp(GetCurrentTimestamp()) << std::endl;
    oss << std::endl;

    std::vector<LoggingSystemHealth> history = GetHealthHistory(time_range_hours * 60);

    if (history.empty()) {
        oss << "No health data available for the specified time range." << std::endl;
        return oss.str();
    }

    // Calculate health metrics
    DWORD total_errors = 0;
    DWORD total_overflows = 0;
    for (const auto& health : history) {
        total_errors += health.error_rate_per_second;
        total_overflows += health.queue_overflow_events;
    }

    oss << "Total Errors: " << total_errors << std::endl;
    oss << "Total Queue Overflows: " << total_overflows << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::GenerateComprehensiveReport(DWORD time_range_hours,
                                                          const std::string& application_name) {
    std::ostringstream oss;

    oss << "=== COMPREHENSIVE SYSTEM REPORT ===" << std::endl;
    oss << "Application: " << application_name << std::endl;
    oss << "Time Range: " << time_range_hours << " hours" << std::endl;
    oss << "Generated: " << FormatTimestamp(GetCurrentTimestamp()) << std::endl;
    oss << std::endl;

    oss << GeneratePerformanceReport(time_range_hours, application_name) << std::endl;
    oss << GenerateHealthReport(time_range_hours) << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::GenerateTrendAnalysisReport(DWORD time_range_hours) {
    std::ostringstream oss;

    oss << "=== TREND ANALYSIS REPORT ===" << std::endl;
    oss << "Time Range: " << time_range_hours << " hours" << std::endl;
    oss << "Generated: " << FormatTimestamp(GetCurrentTimestamp()) << std::endl;
    oss << std::endl;

    PerformanceTrendAnalysis performance_trends = AnalyzePerformanceTrends(time_range_hours * 60);
    PerformanceTrendAnalysis health_trends = AnalyzeHealthTrends(time_range_hours * 60);

    oss << "Performance Trends:" << std::endl;
    oss << "  CPU Usage Trending: " << (performance_trends.cpu_usage_trending_up ? "UP" : "DOWN") << std::endl;
    oss << "  Memory Usage Trending: " << (performance_trends.memory_usage_trending_up ? "UP" : "DOWN") << std::endl;
    oss << std::endl;

    oss << "Health Trends:" << std::endl;
    oss << "  Error Rate Trending: " << (health_trends.error_rate_trending_up ? "UP" : "DOWN") << std::endl;
    oss << "  Message Rate Trending: " << (health_trends.message_rate_trending_up ? "UP" : "DOWN") << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::ExportMetricsToCsv(const std::vector<SystemPerformanceMetrics>& metrics) {
    std::ostringstream oss;

    oss << "Timestamp,CPU_Usage,Memory_Usage,Disk_Usage,Network_In,Network_Out" << std::endl;

    for (const auto& metric : metrics) {
        oss << metric.collection_time << ",";
        oss << metric.cpu_usage_percent << ",";
        oss << metric.memory_usage_percent << ",";
        oss << metric.disk_usage_percent << ",";
        oss << metric.network_bytes_in_per_sec << ",";
        oss << metric.network_bytes_out_per_sec << std::endl;
    }

    return oss.str();
}

std::string MonitoringToolbox::ExportMetricsToJson(const std::vector<SystemPerformanceMetrics>& metrics) {
    std::ostringstream oss;

    oss << "{" << std::endl;
    oss << "  \"metrics\": [" << std::endl;

    for (size_t i = 0; i < metrics.size(); ++i) {
        const auto& metric = metrics[i];
        oss << "    {" << std::endl;
        oss << "      \"timestamp\": " << metric.collection_time << "," << std::endl;
        oss << "      \"cpu_usage\": " << metric.cpu_usage_percent << "," << std::endl;
        oss << "      \"memory_usage\": " << metric.memory_usage_percent << "," << std::endl;
        oss << "      \"disk_usage\": " << metric.disk_usage_percent << std::endl;
        oss << "    }";

        if (i < metrics.size() - 1) {
            oss << ",";
        }
        oss << std::endl;
    }

    oss << "  ]" << std::endl;
    oss << "}" << std::endl;

    return oss.str();
}

// =================================================================================
// SYSTEM HEALTH ASSESSMENT
// =================================================================================

DWORD MonitoringToolbox::AssessSystemHealth(const SystemPerformanceMetrics& current_metrics,
                                           const LoggingSystemHealth& current_health) {
    DWORD cpu_score = 100 - ((current_metrics.cpu_usage_percent < DWORD(100)) ? current_metrics.cpu_usage_percent : DWORD(100));
    DWORD memory_score = 100 - ((current_metrics.memory_usage_percent < DWORD(100)) ? current_metrics.memory_usage_percent : DWORD(100));
    DWORD error_penalty = (current_health.error_rate_per_second * 10 < DWORD(50)) ? current_health.error_rate_per_second * 10 : DWORD(50);

    DWORD overall_score = (cpu_score + memory_score) / 2 - error_penalty;

    DWORD clamped_score = overall_score;
    if (clamped_score > DWORD(100)) clamped_score = DWORD(100);
    return (clamped_score > DWORD(0)) ? clamped_score : DWORD(0);
}

DWORD MonitoringToolbox::AssessLoggingSystemHealth(const std::vector<LoggingSystemHealth>& health_data) {
    if (health_data.empty()) {
        return 50; // Neutral score when no data available
    }

    DWORD total_score = 0;
    for (const auto& health : health_data) {
        DWORD score = AssessSystemHealth({}, health);
        total_score += score;
    }

    return total_score / health_data.size();
}

DWORD MonitoringToolbox::AssessDatabaseHealth(size_t connection_count,
                                             size_t error_count,
                                             DWORD response_time_ms) {
    DWORD score = 100;

    // Penalize for connection issues
    if (connection_count == 0) {
        score -= 50;
    }

    // Penalize for errors
    DWORD error_penalty_db = (error_count * 5 < DWORD(30)) ? error_count * 5 : DWORD(30);
    score -= error_penalty_db;

    // Penalize for slow response times
    if (response_time_ms > 1000) {
        score -= 20;
    } else if (response_time_ms > 500) {
        score -= 10;
    }

    DWORD clamped_score = score;
    if (clamped_score > DWORD(100)) clamped_score = DWORD(100);
    return (clamped_score > DWORD(0)) ? clamped_score : DWORD(0);
}

DWORD MonitoringToolbox::AssessQueueHealth(size_t total_queues,
                                          size_t overflow_events,
                                          DWORD average_latency_ms) {
    DWORD score = 100;

    // Penalize for overflow events
    DWORD overflow_penalty = (overflow_events * 10 < DWORD(50)) ? overflow_events * 10 : DWORD(50);
    score -= overflow_penalty;

    // Penalize for high latency
    if (average_latency_ms > 1000) {
        score -= 30;
    } else if (average_latency_ms > 500) {
        score -= 15;
    }

    DWORD clamped_score_queue = score;
    if (clamped_score_queue > DWORD(100)) clamped_score_queue = DWORD(100);
    return (clamped_score_queue > DWORD(0)) ? clamped_score_queue : DWORD(0);
}

std::vector<std::string> MonitoringToolbox::IdentifyBottlenecks(const SystemPerformanceMetrics& current_metrics) {
    std::vector<std::string> bottlenecks;

    if (current_metrics.cpu_usage_percent > 80) {
        bottlenecks.push_back("High CPU usage detected");
    }

    if (current_metrics.memory_usage_percent > 85) {
        bottlenecks.push_back("High memory usage detected");
    }

    if (current_metrics.disk_usage_percent > 90) {
        bottlenecks.push_back("High disk usage detected");
    }

    return bottlenecks;
}

// =================================================================================
// PREDICTIVE ANALYSIS
// =================================================================================

std::string MonitoringToolbox::PredictMaintenanceNeeds(const SystemPerformanceMetrics& current_metrics,
                                                      DWORD prediction_horizon_hours) {
    std::ostringstream oss;

    DWORD stress_level = CalculateSystemStressLevel(current_metrics, {});

    if (stress_level > 70) {
        oss << "System is under high stress. Maintenance may be needed within ";
        oss << prediction_horizon_hours << " hours.";
    } else if (stress_level > 40) {
        oss << "System is operating normally. Monitor for ";
        oss << (prediction_horizon_hours * 2) << " hours.";
    } else {
        oss << "System is healthy. No immediate maintenance needed.";
    }

    return oss.str();
}

double MonitoringToolbox::PredictQueueOverflow(DWORD current_queue_size,
                                              DWORD message_rate,
                                              DWORD max_queue_size) {
    if (message_rate == 0) {
        return -1.0; // No overflow predicted
    }

    DWORD remaining_capacity = max_queue_size - current_queue_size;
    double hours_to_overflow = static_cast<double>(remaining_capacity) / static_cast<double>(message_rate) * 3600.0;

    return hours_to_overflow;
}

double MonitoringToolbox::PredictMemoryExhaustion(DWORD current_memory_usage,
                                                 double memory_increase_rate,
                                                 DWORD max_memory) {
    if (memory_increase_rate <= 0) {
        return -1.0; // No exhaustion predicted
    }

    DWORD available_memory = max_memory - current_memory_usage;
    double hours_to_exhaustion = static_cast<double>(available_memory) / memory_increase_rate;

    return hours_to_exhaustion;
}

DWORD MonitoringToolbox::CalculateCapacityUtilization(const SystemPerformanceMetrics& current_metrics) {
    DWORD cpu_utilization = current_metrics.cpu_usage_percent;
    DWORD memory_utilization = current_metrics.memory_usage_percent;
    DWORD disk_utilization = current_metrics.disk_usage_percent;

    // Weighted average based on resource importance
    DWORD overall_utilization_capacity = (cpu_utilization * 0.5) + (memory_utilization * 0.3) + (disk_utilization * 0.2);

    return (overall_utilization_capacity < DWORD(100)) ? overall_utilization_capacity : DWORD(100);
}

// =================================================================================
// ADAPTATION STRATEGIES
// =================================================================================

std::vector<std::string> MonitoringToolbox::GenerateCpuOptimizationStrategy(DWORD current_cpu_usage,
                                                                           DWORD target_cpu_usage) {
    std::vector<std::string> strategies;

    if (current_cpu_usage <= target_cpu_usage) {
        strategies.push_back("CPU usage is within acceptable limits");
        return strategies;
    }

    DWORD reduction_needed = current_cpu_usage - target_cpu_usage;

    if (reduction_needed > 50) {
        strategies.push_back("Implement aggressive CPU optimization");
        strategies.push_back("Reduce logging frequency significantly");
        strategies.push_back("Increase batch processing sizes");
    } else if (reduction_needed > 20) {
        strategies.push_back("Implement moderate CPU optimization");
        strategies.push_back("Reduce logging frequency moderately");
        strategies.push_back("Optimize queue processing");
    } else {
        strategies.push_back("Minor CPU optimization needed");
        strategies.push_back("Fine-tune logging configuration");
    }

    return strategies;
}

std::vector<std::string> MonitoringToolbox::GenerateMemoryOptimizationStrategy(DWORD current_memory_usage,
                                                                            DWORD target_memory_usage) {
    std::vector<std::string> strategies;

    if (current_memory_usage <= target_memory_usage) {
        strategies.push_back("Memory usage is within acceptable limits");
        return strategies;
    }

    DWORD reduction_needed = current_memory_usage - target_memory_usage;

    if (reduction_needed > 30) {
        strategies.push_back("Implement aggressive memory optimization");
        strategies.push_back("Reduce queue sizes significantly");
        strategies.push_back("Increase cleanup frequency");
    } else if (reduction_needed > 15) {
        strategies.push_back("Implement moderate memory optimization");
        strategies.push_back("Reduce queue sizes moderately");
        strategies.push_back("Optimize memory allocation");
    } else {
        strategies.push_back("Minor memory optimization needed");
        strategies.push_back("Fine-tune memory settings");
    }

    return strategies;
}

std::vector<std::string> MonitoringToolbox::GenerateQueueOptimizationStrategy(
    const std::vector<SmartQueueStatistics>& current_queue_stats,
    const std::unordered_map<std::string, DWORD>& target_performance) {
    std::vector<std::string> strategies;

    for (const auto& stats : current_queue_stats) {
        if (stats.average_queue_latency_ms > target_performance.at("max_latency_ms")) {
            strategies.push_back("Queue latency too high - consider increasing processing threads");
        }

        if (stats.queue_full_errors > 0) {
            strategies.push_back("Queue full errors detected - consider increasing queue capacity");
        }
    }

    return strategies;
}

std::vector<std::string> MonitoringToolbox::GenerateDatabaseOptimizationStrategy(
    const std::vector<DatabaseOperationResult>& current_db_metrics,
    const std::unordered_map<std::string, DWORD>& target_performance) {
    std::vector<std::string> strategies;

    DWORD total_operations = 0;
    DWORD failed_operations = 0;

    for (const auto& result : current_db_metrics) {
        total_operations++;
        if (!result.success) {
            failed_operations++;
        }
    }

    if (total_operations > 0) {
        DWORD error_rate = (failed_operations * 100) / total_operations;

        if (error_rate > target_performance.at("max_error_rate_percent")) {
            strategies.push_back("High database error rate - consider connection pool tuning");
        }
    }

    return strategies;
}

// =================================================================================
// METRICS CALCULATION
// =================================================================================

double MonitoringToolbox::CalculateMessageThroughput(uint64_t message_count, DWORD time_period_seconds) {
    if (time_period_seconds == 0) {
        return 0.0;
    }

    return static_cast<double>(message_count) / static_cast<double>(time_period_seconds);
}

double MonitoringToolbox::CalculateErrorRate(uint64_t error_count, DWORD time_period_seconds) {
    if (time_period_seconds == 0) {
        return 0.0;
    }

    return static_cast<double>(error_count) / static_cast<double>(time_period_seconds);
}

DWORD MonitoringToolbox::CalculateAverageResponseTime(const std::vector<DWORD>& response_times) {
    if (response_times.empty()) {
        return 0;
    }

    DWORD sum = std::accumulate(response_times.begin(), response_times.end(), DWORD(0));
    return sum / response_times.size();
}

std::unordered_map<std::string, DWORD> MonitoringToolbox::CalculatePerformancePercentiles(
    const std::vector<DWORD>& values) {
    std::unordered_map<std::string, DWORD> percentiles;

    if (values.empty()) {
        return percentiles;
    }

    std::vector<DWORD> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end());

    size_t size = sorted_values.size();

    percentiles["50th"] = sorted_values[size / 2];
    percentiles["90th"] = sorted_values[(size * 90) / 100];
    percentiles["95th"] = sorted_values[(size * 95) / 100];
    percentiles["99th"] = sorted_values[(size * 99) / 100];

    return percentiles;
}

DWORD MonitoringToolbox::CalculateSystemEfficiencyScore(const SystemPerformanceMetrics& metrics) {
    DWORD cpu_efficiency = 100 - ((metrics.cpu_usage_percent < DWORD(100)) ? metrics.cpu_usage_percent : DWORD(100));
    DWORD memory_efficiency = 100 - ((metrics.memory_usage_percent < DWORD(100)) ? metrics.memory_usage_percent : DWORD(100));

    return (cpu_efficiency + memory_efficiency) / 2;
}

// =================================================================================
// CONFIGURATION AND SETUP
// =================================================================================

bool MonitoringToolbox::InitializeMonitoringSystem(DWORD collection_interval_seconds,
                                                  DWORD history_retention_hours) {
    // Initialize monitoring system
    return true;
}

bool MonitoringToolbox::ConfigurePerformanceThresholds(DWORD cpu_warning_threshold,
                                                      DWORD memory_warning_threshold,
                                                      DWORD error_rate_warning_threshold) {
    // Configure thresholds
    return true;
}

bool MonitoringToolbox::ConfigureAlertSystem(const std::string& webhook_url,
                                            const std::string& email_recipients) {
    // Configure alert system
    return true;
}

bool MonitoringToolbox::StartMonitoring() {
    // Start monitoring collection
    return true;
}

bool MonitoringToolbox::StopMonitoring() {
    // Stop monitoring collection
    return true;
}

// =================================================================================
// UTILITY FUNCTIONS
// =================================================================================

std::string MonitoringToolbox::PerformanceMetricsToString(const SystemPerformanceMetrics& metrics) {
    std::ostringstream oss;

    oss << "CPU: " << metrics.cpu_usage_percent << "%" << std::endl;
    oss << "Memory: " << metrics.memory_usage_percent << "%" << std::endl;
    oss << "Disk: " << metrics.disk_usage_percent << "%" << std::endl;
    oss << "Network In: " << FormatBytes(metrics.network_bytes_in_per_sec) << "/sec" << std::endl;
    oss << "Network Out: " << FormatBytes(metrics.network_bytes_out_per_sec) << "/sec" << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::HealthDataToString(const LoggingSystemHealth& health) {
    std::ostringstream oss;

    oss << "Database Connected: " << (health.database_connected ? "Yes" : "No") << std::endl;
    oss << "Message Throughput: " << health.message_throughput_per_second << "/sec" << std::endl;
    oss << "Error Rate: " << health.error_rate_per_second << "/sec" << std::endl;
    oss << "Queue Overflows: " << health.queue_overflow_events << std::endl;
    oss << "Stress Level: " << health.stress_level << "%" << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::StatisticsToString(const ComprehensiveLoggingStatistics& stats) {
    std::ostringstream oss;

    oss << "Total Messages: " << stats.total_messages_logged << std::endl;
    oss << "Average Processing Time: " << stats.average_message_processing_time_ms << "ms" << std::endl;
    oss << "Error Rate: " << stats.error_rate_per_second << "/sec" << std::endl;
    oss << "Peak Memory Usage: " << FormatBytes(stats.peak_memory_usage_mb * 1024 * 1024) << std::endl;

    return oss.str();
}

std::string MonitoringToolbox::TrendAnalysisToString(const PerformanceTrendAnalysis& analysis) {
    std::ostringstream oss;

    oss << "Analysis Period: " << analysis.analysis_period << std::endl;
    oss << "CPU Trend: " << (analysis.cpu_usage_trending_up ? "UP" : "DOWN") << std::endl;
    oss << "Memory Trend: " << (analysis.memory_usage_trending_up ? "UP" : "DOWN") << std::endl;
    oss << "Error Trend: " << (analysis.error_rate_trending_up ? "UP" : "DOWN") << std::endl;

    return oss.str();
}

DWORD MonitoringToolbox::GetCurrentTimestamp() {
    return static_cast<DWORD>(time(nullptr));
}

std::string MonitoringToolbox::FormatDuration(DWORD seconds) {
    std::ostringstream oss;

    DWORD hours = seconds / 3600;
    DWORD minutes = (seconds % 3600) / 60;
    DWORD secs = seconds % 60;

    if (hours > 0) {
        oss << hours << "h ";
    }
    if (minutes > 0) {
        oss << minutes << "m ";
    }
    oss << secs << "s";

    return oss.str();
}

std::string MonitoringToolbox::FormatBytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024 && unit_index < 3) {
        size /= 1024;
        unit_index++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << units[unit_index];
    return oss.str();
}

// =================================================================================
// PRIVATE HELPER METHODS
// =================================================================================

void MonitoringToolbox::CollectWindowsPerformanceMetrics(SystemPerformanceMetrics& metrics) {
    // In a real implementation, this would use Windows Performance Counters
    // For now, simulate realistic values
    metrics.cpu_usage_percent = 45;
    metrics.memory_usage_percent = 60;
    metrics.disk_usage_percent = 30;
    metrics.network_bytes_in_per_sec = 1024 * 100; // 100KB/s
    metrics.network_bytes_out_per_sec = 1024 * 50;  // 50KB/s
}

void MonitoringToolbox::CollectLoggingSystemMetrics(LoggingSystemHealth& health) {
    // Simulate logging system health metrics
    health.database_connected = true;
    health.message_throughput_per_second = 100;
    health.error_rate_per_second = 1;
    health.queue_overflow_events = 0;
    health.stress_level = 25;
}

void MonitoringToolbox::CollectApplicationMetrics(const std::string& application_name,
                                                 ComprehensiveLoggingStatistics& stats) {
    // Simulate application-specific statistics
    stats.total_messages_logged = 100000;
    stats.average_message_processing_time_ms = 5;
    stats.error_rate_per_second = 0.5;
    stats.peak_memory_usage_mb = 256;
}

double MonitoringToolbox::CalculateTrendSlope(const std::vector<double>& values) {
    if (values.size() < 2) {
        return 0.0;
    }

    // Simple linear regression slope calculation
    size_t n = values.size();
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;

    for (size_t i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        double y = values[i];

        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    double denominator = (n * sum_x2) - (sum_x * sum_x);
    if (denominator == 0) {
        return 0.0;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / denominator;
    return slope;
}

std::vector<double> MonitoringToolbox::ExtractMetricValues(const std::vector<SystemPerformanceMetrics>& metrics,
                                                          const std::string& metric_name) {
    std::vector<double> values;

    for (const auto& metric : metrics) {
        if (metric_name == "cpu") {
            values.push_back(static_cast<double>(metric.cpu_usage_percent));
        } else if (metric_name == "memory") {
            values.push_back(static_cast<double>(metric.memory_usage_percent));
        }
    }

    return values;
}

bool MonitoringToolbox::IsAnomalousValue(double value, double mean, double standard_deviation, double threshold) {
    double z_score = std::abs((value - mean) / standard_deviation);
    return z_score > threshold;
}

void MonitoringToolbox::InitializeDefaultThresholds() {
    // Initialize default monitoring thresholds
}

bool MonitoringToolbox::IsInitialized() {
    return !performance_history_.empty() || !health_history_.empty();
}

// Private helper method implementations
uint32_t MonitoringToolbox::GenerateCollectionId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t MonitoringToolbox::GenerateHealthCheckId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t MonitoringToolbox::GenerateStatisticsId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t MonitoringToolbox::GenerateAnalysisId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t MonitoringToolbox::GenerateAdaptationId() {
    static uint32_t next_id = 1;
    return next_id++;
}

std::string MonitoringToolbox::FormatTimestamp(DWORD timestamp) {
    time_t time = static_cast<time_t>(timestamp);
    struct tm tm_time;
    gmtime_s(&tm_time, &time);

    std::ostringstream oss;
    oss << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
}