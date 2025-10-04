/**
 * ASFMLogger Smart Queue Toolbox Implementation
 *
 * Static methods for smart queue operations following toolbox architecture.
 * Pure functions for intelligent message buffering and prioritization.
 */

#include "SmartQueueToolbox.hpp"
#include "../ASFMLoggerCore.hpp"
#include "../structs/LogDataStructures.hpp"
#include "../structs/SmartQueueConfiguration.hpp"
#include "../structs/PersistencePolicy.hpp"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <Windows.h>
#include <ctime>

// Static variables
std::unordered_map<uint32_t, SmartQueueConfiguration> SmartQueueToolbox::queue_configurations_;
std::unordered_map<uint32_t, SmartQueueStatistics> SmartQueueToolbox::queue_statistics_;

// =================================================================================
// QUEUE ENTRY MANAGEMENT
// =================================================================================

QueueEntryMetadata SmartQueueToolbox::CreateQueueEntry(const LogMessageData& message, DWORD priority_score) {
    QueueEntryMetadata entry;
    memset(&entry, 0, sizeof(QueueEntryMetadata));

    entry.message_id = message.message_id;
    entry.queued_time = GetCurrentTimestamp();
    entry.priority_score = priority_score;
    entry.message_size = CalculateMessageSpace(message);
    entry.is_priority_preserved = false;
    entry.preservation_expiry = 0;

    return entry;
}

QueueEntryMetadata SmartQueueToolbox::UpdateQueueEntry(QueueEntryMetadata& entry,
                                                      DWORD new_priority_score,
                                                      bool preserve_priority) {
    if (new_priority_score > 0) {
        entry.priority_score = new_priority_score;
    }

    if (preserve_priority) {
        entry.is_priority_preserved = true;
        entry.preservation_expiry = GetCurrentTimestamp() + 300000; // 5 minutes preservation
    }

    return entry;
}

bool SmartQueueToolbox::IsQueueEntryExpired(const QueueEntryMetadata& entry, DWORD current_time) {
    DWORD max_age_seconds = 3600; // 1 hour default

    // Get max age from configuration if available
    auto it = queue_configurations_.find(entry.message_id);
    if (it != queue_configurations_.end()) {
        max_age_seconds = it->second.max_message_age_seconds;
    }

    DWORD age_seconds = current_time - entry.queued_time;
    return age_seconds > max_age_seconds;
}

bool SmartQueueToolbox::IsPriorityPreservationExpired(const QueueEntryMetadata& entry, DWORD current_time) {
    return entry.is_priority_preserved && current_time > entry.preservation_expiry;
}

// =================================================================================
// PRIORITY CALCULATION
// =================================================================================

DWORD SmartQueueToolbox::CalculatePriorityScore(const LogMessageData& message,
                                               const SmartQueueConfiguration& config,
                                               const PersistenceDecisionContext& context) {
    DWORD base_priority = CalculateBasePriority(message.type, static_cast<MessageImportance>(message.line_number));
    DWORD contextual_bonus = CalculateContextualBonus(message, context);
    DWORD time_priority = CalculateTimePriority(0); // Assume new message

    DWORD total_priority = base_priority + contextual_bonus + time_priority;

    // Apply configuration-based adjustments
    if (config.enable_importance_based_eviction) {
        total_priority *= 2; // Boost priority for importance-based systems
    }

    return total_priority;
}

DWORD SmartQueueToolbox::CalculateBasePriority(LogMessageType message_type, MessageImportance importance) {
    DWORD base_score = 0;

    // Base priority from message type
    switch (message_type) {
        case LogMessageType::CRITICAL_LOG:
            base_score += 100;
            break;
        case LogMessageType::ERR:
            base_score += 80;
            break;
        case LogMessageType::WARN:
            base_score += 60;
            break;
        case LogMessageType::INFO:
            base_score += 40;
            break;
        case LogMessageType::DEBUG:
            base_score += 20;
            break;
        case LogMessageType::TRACE:
            base_score += 10;
            break;
        default:
            base_score += 30;
            break;
    }

    // Boost from importance level
    switch (importance) {
        case MessageImportance::CRITICAL:
            base_score += 50;
            break;
        case MessageImportance::HIGH:
            base_score += 30;
            break;
        case MessageImportance::MEDIUM:
            base_score += 15;
            break;
        case MessageImportance::LOW:
            base_score += 5;
            break;
        default:
            break;
    }

    return base_score;
}

DWORD SmartQueueToolbox::CalculateContextualBonus(const LogMessageData& message,
                                                 const PersistenceDecisionContext& context) {
    DWORD bonus = 0;

    // Bonus for application context
    if (strlen(context.application_name) > 0) {
        bonus += 10;
    }

    // Bonus for system stress
    if (context.current_system_load > 70) {
        bonus += 20;
    }

    // Bonus for recent errors
    if (context.current_error_rate > 5) {
        bonus += 15;
    }

    return bonus;
}

DWORD SmartQueueToolbox::CalculateTimePriority(DWORD message_age_seconds) {
    // Newer messages get slight priority boost
    if (message_age_seconds < 60) { // Less than 1 minute old
        return 10;
    } else if (message_age_seconds < 300) { // Less than 5 minutes old
        return 5;
    }

    return 0;
}

// =================================================================================
// QUEUE SPACE MANAGEMENT
// =================================================================================

bool SmartQueueToolbox::CanMessageFit(const LogMessageData& message,
                                     size_t current_queue_size,
                                     const SmartQueueConfiguration& config) {
    size_t message_space = CalculateMessageSpace(message);
    size_t available_space = config.max_total_size - current_queue_size;

    return message_space <= available_space;
}

size_t SmartQueueToolbox::CalculateMessageSpace(const LogMessageData& message) {
    return strlen(message.message) + strlen(message.component) +
           strlen(message.function) + strlen(message.file) + 256; // Overhead
}

bool SmartQueueToolbox::HasReservedSpace(MessageImportance importance,
                                        const std::unordered_map<MessageImportance, size_t>& current_usage,
                                        const SmartQueueConfiguration& config) {
    size_t reserved_space = 0;

    switch (importance) {
        case MessageImportance::CRITICAL:
            reserved_space = config.critical_messages_reserved;
            break;
        case MessageImportance::HIGH:
            reserved_space = config.high_messages_reserved;
            break;
        case MessageImportance::MEDIUM:
            reserved_space = config.medium_messages_reserved;
            break;
        case MessageImportance::LOW:
            reserved_space = config.low_messages_reserved;
            break;
        default:
            return true; // No reservation requirement
    }

    auto it = current_usage.find(importance);
    if (it == current_usage.end()) {
        return true; // No usage tracked yet
    }

    return it->second < reserved_space;
}

void SmartQueueToolbox::UpdateSpaceUsage(std::unordered_map<MessageImportance, size_t>& current_usage,
                                        MessageImportance importance,
                                        size_t space_used,
                                        bool is_adding) {
    if (is_adding) {
        current_usage[importance] += space_used;
    } else {
        auto it = current_usage.find(importance);
        if (it != current_usage.end() && it->second >= space_used) {
            it->second -= space_used;
        }
    }
}

// =================================================================================
// INTELLIGENT EVICTION STRATEGIES
// =================================================================================

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::FindMessagesToEvict(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    size_t space_needed,
    const SmartQueueConfiguration& config) {
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> messages_to_evict;

    if (config.enable_importance_based_eviction) {
        messages_to_evict = FindLowestPriorityMessages(queue, 10);
    } else if (config.enable_time_based_eviction) {
        messages_to_evict = FindOldestMessages(queue, 10, GetCurrentTimestamp());
    } else {
        messages_to_evict = FindNonPreservedMessages(queue, 10, GetCurrentTimestamp());
    }

    return messages_to_evict;
}

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::FindOldestMessages(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    size_t count,
    DWORD current_time) {
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> candidates;

    for (const auto& pair : queue) {
        DWORD age_seconds = current_time - pair.second.queued_time;
        if (age_seconds > 300) { // Older than 5 minutes
            candidates.push_back(pair);
        }
    }

    // Sort by age (oldest first)
    std::sort(candidates.begin(), candidates.end(),
              [current_time](const auto& a, const auto& b) {
                  DWORD age_a = current_time - a.second.queued_time;
                  DWORD age_b = current_time - b.second.queued_time;
                  return age_a > age_b;
              });

    // Return only the requested count
    if (candidates.size() > count) {
        candidates.resize(count);
    }

    return candidates;
}

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::FindLowestPriorityMessages(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    size_t count) {
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> candidates = queue;

    // Sort by priority (lowest first)
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) {
                  return a.second.priority_score < b.second.priority_score;
              });

    // Return only the requested count
    if (candidates.size() > count) {
        candidates.resize(count);
    }

    return candidates;
}

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::FindNonPreservedMessages(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    size_t count,
    DWORD current_time) {
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> candidates;

    for (const auto& pair : queue) {
        if (!pair.second.is_priority_preserved ||
            IsPriorityPreservationExpired(pair.second, current_time)) {
            candidates.push_back(pair);
        }
    }

    // Return only the requested count
    if (candidates.size() > count) {
        candidates.resize(count);
    }

    return candidates;
}

std::vector<QueueEvictionDecision> SmartQueueToolbox::ExecuteEviction(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& messages_to_evict,
    const std::string& reason) {
    std::vector<QueueEvictionDecision> decisions;

    for (const auto& pair : messages_to_evict) {
        QueueEvictionDecision decision;
        memset(&decision, 0, sizeof(QueueEvictionDecision));

        decision.message_id = pair.first.message_id;
        decision.eviction_time = GetCurrentTimestamp();
        strcpy_s(decision.eviction_reason, reason.c_str());
        strcpy_s(decision.eviction_method, "PRIORITY");
        decision.queue_size_before = 1000; // Would get from actual queue
        decision.queue_size_after = 999;
        decision.memory_freed_bytes = CalculateMessageSpace(pair.first);

        decisions.push_back(decision);
    }

    return decisions;
}

// =================================================================================
// PRIORITY PRESERVATION
// =================================================================================

std::vector<QueueEntryMetadata> SmartQueueToolbox::MarkForPriorityPreservation(
    const std::vector<LogMessageData>& messages,
    DWORD preservation_duration_ms) {
    std::vector<QueueEntryMetadata> updated_entries;

    DWORD expiry_time = GetCurrentTimestamp() + (preservation_duration_ms / 1000);

    for (const auto& message : messages) {
        QueueEntryMetadata entry;
        memset(&entry, 0, sizeof(QueueEntryMetadata));

        entry.message_id = message.message_id;
        entry.is_priority_preserved = true;
        entry.preservation_expiry = expiry_time;

        updated_entries.push_back(entry);
    }

    return updated_entries;
}

std::vector<QueueEntryMetadata> SmartQueueToolbox::ExtendPriorityPreservation(
    const std::vector<QueueEntryMetadata>& entries,
    DWORD additional_duration_ms) {
    std::vector<QueueEntryMetadata> updated_entries = entries;
    DWORD additional_seconds = additional_duration_ms / 1000;

    for (auto& entry : updated_entries) {
        entry.preservation_expiry += additional_seconds;
    }

    return updated_entries;
}

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::FindPreservedMessages(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    size_t eviction_count,
    DWORD current_time) {
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> preserved_messages;

    for (const auto& pair : queue) {
        if (pair.second.is_priority_preserved &&
            !IsPriorityPreservationExpired(pair.second, current_time)) {
            preserved_messages.push_back(pair);

            if (preserved_messages.size() >= eviction_count) {
                break;
            }
        }
    }

    return preserved_messages;
}

bool SmartQueueToolbox::ShouldPreserveMessage(const QueueEntryMetadata& entry,
                                             DWORD current_time,
                                             const SmartQueueConfiguration& config) {
    if (!config.enable_priority_preservation) {
        return false;
    }

    return entry.is_priority_preserved &&
           !IsPriorityPreservationExpired(entry, current_time) &&
           entry.priority_score >= 80; // High priority threshold
}

// =================================================================================
// BATCH OPERATIONS
// =================================================================================

QueueBatch SmartQueueToolbox::CreateOptimalBatch(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    const SmartQueueConfiguration& config,
    size_t max_batch_size) {
    QueueBatch batch;
    memset(&batch, 0, sizeof(QueueBatch));

    batch.batch_id = GenerateBatchId();
    batch.created_time = GetCurrentTimestamp();
    batch.message_count = 0;
    batch.total_size_bytes = 0;
    batch.max_age_in_batch_ms = 0;
    batch.is_priority_batch = false;
    strcpy_s(batch.batch_type, "OPTIMAL");

    // Select messages for batch based on priority and size
    size_t current_size = 0;
    DWORD oldest_message_time = UINT_MAX;

    for (const auto& pair : queue) {
        if (batch.message_count >= max_batch_size) {
            break;
        }

        size_t message_size = CalculateMessageSpace(pair.first);
        if (current_size + message_size > config.max_bulk_size) {
            continue; // Skip if message would exceed batch size limit
        }

        // Add message to batch
        current_size += message_size;
        batch.message_count++;
        batch.total_size_bytes += message_size;

        DWORD message_age_ms = (GetCurrentTimestamp() - pair.second.queued_time) * 1000;
        if (message_age_ms > batch.max_age_in_batch_ms) {
            batch.max_age_in_batch_ms = message_age_ms;
        }

        if (oldest_message_time == UINT_MAX || pair.second.queued_time < oldest_message_time) {
            oldest_message_time = pair.second.queued_time;
        }
    }

    return batch;
}

QueueBatch SmartQueueToolbox::CreatePriorityBatch(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    const SmartQueueConfiguration& config) {
    QueueBatch batch;
    memset(&batch, 0, sizeof(QueueBatch));

    batch.batch_id = GenerateBatchId();
    batch.created_time = GetCurrentTimestamp();
    batch.is_priority_batch = true;
    strcpy_s(batch.batch_type, "PRIORITY");

    // Select only high-priority messages
    for (const auto& pair : queue) {
        if (pair.second.priority_score >= 80) { // High priority threshold
            batch.message_count++;
            batch.total_size_bytes += CalculateMessageSpace(pair.first);

            DWORD message_age_ms = (GetCurrentTimestamp() - pair.second.queued_time) * 1000;
            if (message_age_ms > batch.max_age_in_batch_ms) {
                batch.max_age_in_batch_ms = message_age_ms;
            }
        }
    }

    return batch;
}

QueueBatch SmartQueueToolbox::CreateAgeBasedBatch(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
    const SmartQueueConfiguration& config,
    DWORD current_time) {
    QueueBatch batch;
    memset(&batch, 0, sizeof(QueueBatch));

    batch.batch_id = GenerateBatchId();
    batch.created_time = GetCurrentTimestamp();
    strcpy_s(batch.batch_type, "AGE_BASED");

    // Select oldest messages first
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> sorted_queue = queue;
    std::sort(sorted_queue.begin(), sorted_queue.end(),
              [current_time](const auto& a, const auto& b) {
                  return a.second.queued_time < b.second.queued_time;
              });

    for (const auto& pair : sorted_queue) {
        DWORD age_seconds = current_time - pair.second.queued_time;
        if (age_seconds > 300) { // Older than 5 minutes
            batch.message_count++;
            batch.total_size_bytes += CalculateMessageSpace(pair.first);

            DWORD message_age_ms = age_seconds * 1000;
            if (message_age_ms > batch.max_age_in_batch_ms) {
                batch.max_age_in_batch_ms = message_age_ms;
            }
        }
    }

    return batch;
}

bool SmartQueueToolbox::ValidateBatch(const QueueBatch& batch) {
    return batch.batch_id != 0 &&
           batch.message_count > 0 &&
           batch.total_size_bytes > 0 &&
           strlen(batch.batch_type) > 0;
}

// =================================================================================
// QUEUE ORGANIZATION AND SORTING
// =================================================================================

void SmartQueueToolbox::SortByPriority(std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue) {
    std::sort(queue.begin(), queue.end(),
              [](const auto& a, const auto& b) {
                  return a.second.priority_score > b.second.priority_score;
              });
}

void SmartQueueToolbox::SortByAge(std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue,
                                 DWORD current_time) {
    std::sort(queue.begin(), queue.end(),
              [current_time](const auto& a, const auto& b) {
                  DWORD age_a = current_time - a.second.queued_time;
                  DWORD age_b = current_time - b.second.queued_time;
                  return age_a < age_b; // Older messages first
              });
}

void SmartQueueToolbox::SortBySize(std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue) {
    std::sort(queue.begin(), queue.end(),
              [](const auto& a, const auto& b) {
                  size_t size_a = CalculateMessageSpace(a.first);
                  size_t size_b = CalculateMessageSpace(b.first);
                  return size_a > size_b; // Larger messages first
              });
}

std::unordered_map<MessageImportance, std::vector<LogMessageData>> SmartQueueToolbox::GroupByImportance(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue) {
    std::unordered_map<MessageImportance, std::vector<LogMessageData>> grouped;

    for (const auto& pair : queue) {
        MessageImportance importance = static_cast<MessageImportance>(pair.first.line_number);
        grouped[importance].push_back(pair.first);
    }

    return grouped;
}

std::unordered_map<LogMessageType, std::vector<LogMessageData>> SmartQueueToolbox::GroupByMessageType(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& queue) {
    std::unordered_map<LogMessageType, std::vector<LogMessageData>> grouped;

    for (const auto& pair : queue) {
        grouped[pair.first.type].push_back(pair.first);
    }

    return grouped;
}

// =================================================================================
// STATISTICS AND MONITORING
// =================================================================================

void SmartQueueToolbox::UpdateQueueStatistics(uint32_t queue_id,
                                             const std::string& operation,
                                             size_t message_count,
                                             size_t bytes_affected) {
    auto it = queue_statistics_.find(queue_id);
    if (it == queue_statistics_.end()) {
        return; // Statistics not initialized for this queue
    }

    SmartQueueStatistics& stats = it->second;

    if (operation == "QUEUE") {
        stats.total_messages_queued += message_count;
    } else if (operation == "DEQUEUE") {
        stats.total_messages_dequeued += message_count;
    } else if (operation == "EVICT") {
        stats.total_messages_evicted += message_count;
    }

    stats.current_queue_size += message_count; // Simplified tracking
}

SmartQueueStatistics SmartQueueToolbox::GetQueueStatistics(uint32_t queue_id) {
    auto it = queue_statistics_.find(queue_id);
    if (it != queue_statistics_.end()) {
        return it->second;
    }

    // Return empty statistics if not found
    SmartQueueStatistics empty_stats;
    memset(&empty_stats, 0, sizeof(SmartQueueStatistics));
    return empty_stats;
}

void SmartQueueToolbox::ResetQueueStatistics(uint32_t queue_id) {
    auto it = queue_statistics_.find(queue_id);
    if (it != queue_statistics_.end()) {
        memset(&it->second, 0, sizeof(SmartQueueStatistics));
        it->second.queue_id = queue_id;
    }
}

std::string SmartQueueToolbox::AnalyzeQueuePerformance(uint32_t queue_id) {
    std::ostringstream oss;

    SmartQueueStatistics stats = GetQueueStatistics(queue_id);

    oss << "Queue Performance Analysis:" << std::endl;
    oss << "Total Messages Queued: " << stats.total_messages_queued << std::endl;
    oss << "Total Messages Dequeued: " << stats.total_messages_dequeued << std::endl;
    oss << "Total Messages Evicted: " << stats.total_messages_evicted << std::endl;
    oss << "Current Queue Size: " << stats.current_queue_size << std::endl;
    oss << "Average Latency: " << stats.average_queue_latency_ms << "ms" << std::endl;

    return oss.str();
}

std::vector<std::pair<std::string, double>> SmartQueueToolbox::CalculateEfficiencyMetrics(uint32_t queue_id) {
    std::vector<std::pair<std::string, double>> metrics;

    SmartQueueStatistics stats = GetQueueStatistics(queue_id);

    if (stats.total_messages_queued > 0) {
        double dequeue_rate = static_cast<double>(stats.total_messages_dequeued) / static_cast<double>(stats.total_messages_queued);
        metrics.push_back({"Dequeue Rate", dequeue_rate});

        double eviction_rate = static_cast<double>(stats.total_messages_evicted) / static_cast<double>(stats.total_messages_queued);
        metrics.push_back({"Eviction Rate", eviction_rate});

        double throughput = static_cast<double>(stats.queue_throughput_per_second);
        metrics.push_back({"Throughput (msg/sec)", throughput});
    }

    return metrics;
}

// =================================================================================
// CONFIGURATION MANAGEMENT
// =================================================================================

bool SmartQueueToolbox::SetQueueConfiguration(uint32_t queue_id, const SmartQueueConfiguration& config) {
    queue_configurations_[queue_id] = config;

    // Initialize statistics for this queue
    SmartQueueStatistics stats;
    memset(&stats, 0, sizeof(SmartQueueStatistics));
    stats.queue_id = queue_id;
    stats.collection_period_seconds = 3600; // 1 hour
    queue_statistics_[queue_id] = stats;

    return true;
}

SmartQueueConfiguration SmartQueueToolbox::GetQueueConfiguration(uint32_t queue_id) {
    auto it = queue_configurations_.find(queue_id);
    if (it != queue_configurations_.end()) {
        return it->second;
    }

    // Return default configuration if not found
    return CreateDefaultConfiguration("DefaultQueue", "DefaultApp");
}

SmartQueueConfiguration SmartQueueToolbox::CreateDefaultConfiguration(const std::string& queue_name,
                                                                     const std::string& application_name) {
    SmartQueueConfiguration config;
    memset(&config, 0, sizeof(SmartQueueConfiguration));

    config.queue_id = GenerateQueueId();
    strcpy_s(config.queue_name, queue_name.c_str());
    strcpy_s(config.application_name, application_name.c_str());

    // Size limits
    config.max_total_size = 100 * 1024 * 1024; // 100MB
    config.max_priority_size = 10 * 1024 * 1024; // 10MB
    config.max_normal_size = 80 * 1024 * 1024; // 80MB
    config.max_bulk_size = 5 * 1024 * 1024; // 5MB

    // Importance-based sizing
    config.critical_messages_reserved = 1024 * 1024; // 1MB
    config.high_messages_reserved = 512 * 1024; // 512KB
    config.medium_messages_reserved = 256 * 1024; // 256KB
    config.low_messages_reserved = 128 * 1024; // 128KB

    // Eviction policies
    config.enable_importance_based_eviction = true;
    config.enable_time_based_eviction = true;
    config.enable_size_based_eviction = false;
    config.max_message_age_seconds = 3600; // 1 hour

    // Performance settings
    config.enable_batch_processing = true;
    config.batch_size = 100;
    config.batch_timeout_ms = 5000;

    // Priority preservation
    config.enable_priority_preservation = true;
    config.priority_preservation_count = 50;
    config.priority_preservation_duration_ms = 300000; // 5 minutes

    // Monitoring
    config.enable_detailed_statistics = true;
    config.statistics_collection_interval_ms = 60000; // 1 minute

    // Threading
    config.enable_lock_free_operations = false;
    config.spin_lock_timeout_ms = 1000;

    // Lifecycle
    config.created_time = GetCurrentTimestamp();
    config.last_modified = config.created_time;
    strcpy_s(config.created_by, "SmartQueueToolbox");
    config.is_active = true;

    return config;
}

SmartQueueConfiguration SmartQueueToolbox::CreateHighPerformanceConfiguration(const std::string& queue_name,
                                                                            const std::string& application_name) {
    SmartQueueConfiguration config = CreateDefaultConfiguration(queue_name, application_name);

    strcpy_s(config.queue_name, "HighPerformance");
    strcpy_s(config.application_name, application_name.c_str());

    // Optimize for performance
    config.max_total_size = 500 * 1024 * 1024; // 500MB
    config.batch_size = 500;
    config.batch_timeout_ms = 1000;
    config.enable_lock_free_operations = true;
    config.enable_detailed_statistics = false; // Disable for performance

    return config;
}

bool SmartQueueToolbox::ValidateQueueConfiguration(const SmartQueueConfiguration& config) {
    return config.queue_id != 0 &&
           strlen(config.queue_name) > 0 &&
           strlen(config.application_name) > 0 &&
           config.max_total_size > 0 &&
           config.is_active;
}

// =================================================================================
// MEMORY MANAGEMENT
// =================================================================================

DWORD SmartQueueToolbox::CalculateMemoryPressure(size_t current_usage, size_t max_capacity) {
    if (max_capacity == 0) {
        return 100;
    }

    DWORD utilization_percent = static_cast<DWORD>((current_usage * 100) / max_capacity);
    return (utilization_percent < DWORD(100)) ? utilization_percent : DWORD(100);
}

bool SmartQueueToolbox::IsMemoryPressureCritical(size_t current_usage,
                                                size_t max_capacity,
                                                DWORD pressure_threshold) {
    DWORD pressure = CalculateMemoryPressure(current_usage, max_capacity);
    return pressure >= pressure_threshold;
}

size_t SmartQueueToolbox::CalculateOptimalMemoryAllocation(DWORD message_rate,
                                                          size_t average_message_size,
                                                          DWORD retention_time_seconds) {
    // Calculate memory needed for retention period
    size_t messages_in_retention = message_rate * retention_time_seconds;
    size_t total_memory_needed = messages_in_retention * average_message_size;

    // Add overhead (50% for metadata and fragmentation)
    size_t total_allocation = total_memory_needed * 1.5;

    // Reasonable limits
    if (total_allocation < 1024 * 1024) { // Minimum 1MB
        total_allocation = 1024 * 1024;
    }
    if (total_allocation > 1024 * 1024 * 1024) { // Maximum 1GB
        total_allocation = 1024 * 1024 * 1024;
    }

    return total_allocation;
}

// =================================================================================
// UTILITY FUNCTIONS
// =================================================================================

std::string SmartQueueToolbox::EvictionDecisionToString(const QueueEvictionDecision& decision) {
    std::ostringstream oss;

    oss << "Eviction Decision:" << std::endl;
    oss << "Message ID: " << decision.message_id << std::endl;
    oss << "Reason: " << decision.eviction_reason << std::endl;
    oss << "Method: " << decision.eviction_method << std::endl;
    oss << "Memory Freed: " << decision.memory_freed_bytes << " bytes" << std::endl;

    return oss.str();
}

std::string SmartQueueToolbox::QueueConfigurationToString(const SmartQueueConfiguration& config) {
    std::ostringstream oss;

    oss << "Queue Configuration:" << std::endl;
    oss << "Name: " << config.queue_name << std::endl;
    oss << "Application: " << config.application_name << std::endl;
    oss << "Max Size: " << config.max_total_size << " bytes" << std::endl;
    oss << "Batch Size: " << config.batch_size << std::endl;
    oss << "Priority Preservation: " << (config.enable_priority_preservation ? "Enabled" : "Disabled") << std::endl;

    return oss.str();
}

std::string SmartQueueToolbox::QueueStatisticsToString(const SmartQueueStatistics& stats) {
    std::ostringstream oss;

    oss << "Queue Statistics:" << std::endl;
    oss << "Total Queued: " << stats.total_messages_queued << std::endl;
    oss << "Total Dequeued: " << stats.total_messages_dequeued << std::endl;
    oss << "Total Evicted: " << stats.total_messages_evicted << std::endl;
    oss << "Current Size: " << stats.current_queue_size << std::endl;
    oss << "Average Latency: " << stats.average_queue_latency_ms << "ms" << std::endl;

    return oss.str();
}

DWORD SmartQueueToolbox::GetCurrentTimestamp() {
    return static_cast<DWORD>(time(nullptr));
}

// =================================================================================
// PRIVATE HELPER METHODS
// =================================================================================

uint32_t SmartQueueToolbox::GenerateQueueId() {
    static uint32_t next_id = 1;
    return next_id++;
}

uint32_t SmartQueueToolbox::GenerateBatchId() {
    static uint32_t next_id = 1;
    return next_id++;
}

DWORD SmartQueueToolbox::CalculateMessageAge(const QueueEntryMetadata& entry, DWORD current_time) {
    return current_time - entry.queued_time;
}

bool SmartQueueToolbox::ComparePriority(const std::pair<LogMessageData, QueueEntryMetadata>& a,
                                       const std::pair<LogMessageData, QueueEntryMetadata>& b) {
    return a.second.priority_score > b.second.priority_score;
}

bool SmartQueueToolbox::CompareAge(const std::pair<LogMessageData, QueueEntryMetadata>& a,
                                  const std::pair<LogMessageData, QueueEntryMetadata>& b,
                                  DWORD current_time) {
    DWORD age_a = current_time - a.second.queued_time;
    DWORD age_b = current_time - b.second.queued_time;
    return age_a < age_b; // Older first
}

bool SmartQueueToolbox::CompareMessages(const LogMessageData& a, const LogMessageData& b) {
    return a.message_id == b.message_id &&
           a.type == b.type &&
           strcmp(a.message, b.message) == 0 &&
           strcmp(a.component, b.component) == 0 &&
           strcmp(a.function, b.function) == 0;
}

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::DequeToVector(
    const std::deque<std::pair<LogMessageData, QueueEntryMetadata>>& deque_container) {
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> result;
    result.assign(deque_container.begin(), deque_container.end());
    return result;
}

std::deque<std::pair<LogMessageData, QueueEntryMetadata>> SmartQueueToolbox::VectorToDeque(
    const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& vector_container) {
    std::deque<std::pair<LogMessageData, QueueEntryMetadata>> result;
    result.assign(vector_container.begin(), vector_container.end());
    return result;
}

size_t SmartQueueToolbox::EstimateBatchSize(const std::vector<std::pair<LogMessageData, QueueEntryMetadata>>& candidates,
                                           size_t max_size) {
    size_t total_size = 0;
    size_t count = 0;

    for (const auto& candidate : candidates) {
        size_t message_size = CalculateMessageSpace(candidate.first);
        if (total_size + message_size > max_size) {
            break;
        }

        total_size += message_size;
        count++;
    }

    return count;
}

void SmartQueueToolbox::InitializeDefaultConfigurations() {
    // Initialize default configurations if needed
}

bool SmartQueueToolbox::IsInitialized() {
    return !queue_configurations_.empty();
}