/**
 * ASFMLogger SmartMessageQueue Implementation
 *
 * Stateful wrapper implementation for intelligent message queuing.
 * Provides thread-safe object-oriented interface using SmartQueueToolbox internally.
 */

#include "stateful/SmartMessageQueue.hpp"
#include "toolbox/SmartQueueToolbox.hpp"
#include "toolbox/ImportanceToolbox.hpp"
#include "structs/SmartQueueConfiguration.hpp"
#include "ASFMLoggerCore.hpp"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <functional>

// =====================================================================================
// CONSTRUCTORS AND DESTRUCTOR
// =====================================================================================

SmartMessageQueue::SmartMessageQueue()
    : queue_id_(0), config_(), message_queue_(), space_usage_by_importance_(),
      queue_mutex_(), queue_condition_(),
      total_messages_queued_(0), total_messages_dequeued_(0), total_messages_evicted_(0),
      current_memory_usage_(0), preserved_messages_(), preservation_mutex_(),
      current_batch_(), batch_mutex_(), last_batch_time_(0) {

    // Initialize with default configuration
    initializeWithConfiguration(SmartQueueToolbox::CreateDefaultConfiguration("DefaultQueue", "DefaultApp"));
}

SmartMessageQueue::SmartMessageQueue(const SmartQueueConfiguration& config)
    : queue_id_(0), config_(config), message_queue_(), space_usage_by_importance_(),
      queue_mutex_(), queue_condition_(),
      total_messages_queued_(0), total_messages_dequeued_(0), total_messages_evicted_(0),
      current_memory_usage_(0), preserved_messages_(), preservation_mutex_(),
      current_batch_(), batch_mutex_(), last_batch_time_(0) {

    // Initialize with provided configuration
    initializeWithConfiguration(config);
}

SmartMessageQueue::SmartMessageQueue(const std::string& queue_name,
                                   const std::string& application_name,
                                   size_t max_size)
    : queue_id_(0), config_(), message_queue_(), space_usage_by_importance_(),
      queue_mutex_(), queue_condition_(),
      total_messages_queued_(0), total_messages_dequeued_(0), total_messages_evicted_(0),
      current_memory_usage_(0), preserved_messages_(), preservation_mutex_(),
      current_batch_(), batch_mutex_(), last_batch_time_(0) {

    // Create configuration with specified parameters
    config_ = SmartQueueToolbox::CreateDefaultConfiguration(queue_name, application_name);
    config_.max_total_size = max_size;

    // Initialize with configuration
    initializeWithConfiguration(config_);
}

SmartMessageQueue::SmartMessageQueue(const SmartMessageQueue& other)
    : queue_id_(other.queue_id_), config_(other.config_), message_queue_(other.message_queue_),
      space_usage_by_importance_(other.space_usage_by_importance_), queue_mutex_(), queue_condition_(),
      total_messages_queued_(other.total_messages_queued_.load()),
      total_messages_dequeued_(other.total_messages_dequeued_.load()),
      total_messages_evicted_(other.total_messages_evicted_.load()),
      current_memory_usage_(other.current_memory_usage_.load()),
      preserved_messages_(), preservation_mutex_(),
      current_batch_(), batch_mutex_(), last_batch_time_(other.last_batch_time_) {

    // Copy preserved messages with proper locking
    {
        std::lock_guard<std::mutex> lock(other.preservation_mutex_);
        preserved_messages_ = other.preserved_messages_;
    }
}

SmartMessageQueue::SmartMessageQueue(SmartMessageQueue&& other) noexcept
    : queue_id_(other.queue_id_), config_(std::move(other.config_)),
      message_queue_(std::move(other.message_queue_)),
      space_usage_by_importance_(std::move(other.space_usage_by_importance_)),
      queue_mutex_(), queue_condition_(),
      total_messages_queued_(other.total_messages_queued_.load()),
      total_messages_dequeued_(other.total_messages_dequeued_.load()),
      total_messages_evicted_(other.total_messages_evicted_.load()),
      current_memory_usage_(other.current_memory_usage_.load()),
      preserved_messages_(std::move(other.preserved_messages_)),
      preservation_mutex_(),
      current_batch_(std::move(other.current_batch_)),
      batch_mutex_(), last_batch_time_(other.last_batch_time_) {
    // Move constructor - data is already moved
}

SmartMessageQueue& SmartMessageQueue::operator=(const SmartMessageQueue& other) {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(queue_mutex_);
        std::lock_guard<std::mutex> other_lock(other.queue_mutex_);
        std::lock_guard<std::mutex> this_preserve_lock(preservation_mutex_);
        std::lock_guard<std::mutex> other_preserve_lock(other.preservation_mutex_);

        queue_id_ = other.queue_id_;
        config_ = other.config_;
        message_queue_ = other.message_queue_;
        space_usage_by_importance_ = other.space_usage_by_importance_;
        total_messages_queued_ = other.total_messages_queued_.load();
        total_messages_dequeued_ = other.total_messages_dequeued_.load();
        total_messages_evicted_ = other.total_messages_evicted_.load();
        current_memory_usage_ = other.current_memory_usage_.load();
        preserved_messages_ = other.preserved_messages_;
        current_batch_ = other.current_batch_;
        last_batch_time_ = other.last_batch_time_;
    }
    return *this;
}

SmartMessageQueue& SmartMessageQueue::operator=(SmartMessageQueue&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> this_lock(queue_mutex_);
        std::lock_guard<std::mutex> this_preserve_lock(preservation_mutex_);

        queue_id_ = other.queue_id_;
        config_ = std::move(other.config_);
        message_queue_ = std::move(other.message_queue_);
        space_usage_by_importance_ = std::move(other.space_usage_by_importance_);
        total_messages_queued_ = other.total_messages_queued_.load();
        total_messages_dequeued_ = other.total_messages_dequeued_.load();
        total_messages_evicted_ = other.total_messages_evicted_.load();
        current_memory_usage_ = other.current_memory_usage_.load();
        preserved_messages_ = std::move(other.preserved_messages_);
        current_batch_ = std::move(other.current_batch_);
        last_batch_time_ = other.last_batch_time_;
        // other data is left in valid but unspecified state
    }
    return *this;
}

SmartMessageQueue::~SmartMessageQueue() {
    // Clean shutdown - no special cleanup needed for ToolBox delegation
}

// =====================================================================================
// PRIVATE HELPER METHODS
// =====================================================================================

void SmartMessageQueue::initializeWithConfiguration(const SmartQueueConfiguration& config) {
    // Set configuration using ToolBox
    SmartQueueToolbox::SetQueueConfiguration(queue_id_, config);

    // Generate unique queue ID using hash of queue name and current time
    queue_id_ = std::hash<std::string>{}(config.queue_name) + GetCurrentTime();

    // Initialize space usage tracking
    space_usage_by_importance_[MessageImportance::LOW] = 0;
    space_usage_by_importance_[MessageImportance::MEDIUM] = 0;
    space_usage_by_importance_[MessageImportance::HIGH] = 0;
    space_usage_by_importance_[MessageImportance::CRITICAL] = 0;

    // Reset statistics
    resetStatistics();
}

bool SmartMessageQueue::canAcceptMessage(const LogMessageData& message,
                                         const PersistenceDecisionContext& context) {
    // Check if queue is full
    if (message_queue_.size() >= config_.max_total_size) {
        return false;
    }

    // Check if message can fit
    if (!SmartQueueToolbox::CanMessageFit(message, message_queue_.size(), config_)) {
        return false;
    }

    // Check if we have reserved space for this importance level
    MessageImportance importance = determineMessageImportance(message, context);
    if (!SmartQueueToolbox::HasReservedSpace(importance, space_usage_by_importance_, config_)) {
        return false;
    }

    return true;
}

void SmartMessageQueue::updateSpaceUsage(const LogMessageData& message,
                                        MessageImportance importance,
                                        bool is_adding) {
    size_t message_space = SmartQueueToolbox::CalculateMessageSpace(message);

    SmartQueueToolbox::UpdateSpaceUsage(space_usage_by_importance_, importance, message_space, is_adding);

    // Update total memory usage
    if (is_adding) {
        current_memory_usage_ += message_space;
    } else {
        current_memory_usage_ = (current_memory_usage_ > message_space) ?
                               current_memory_usage_ - message_space : 0;
    }
}

void SmartMessageQueue::updateQueueStatistics(const std::string& operation,
                                             size_t message_count,
                                             size_t bytes_affected) {
    // Update ToolBox statistics
    SmartQueueToolbox::UpdateQueueStatistics(queue_id_, operation, message_count, bytes_affected);

    // Update atomic counters
    if (operation == "QUEUE") {
        total_messages_queued_ += message_count;
    } else if (operation == "DEQUEUE") {
        total_messages_dequeued_ += message_count;
    } else if (operation == "EVICT") {
        total_messages_evicted_ += message_count;
    }
}

size_t SmartMessageQueue::calculateMessageSpace(const LogMessageData& message) const {
    return SmartQueueToolbox::CalculateMessageSpace(message);
}

MessageImportance SmartMessageQueue::determineMessageImportance(const LogMessageData& message,
                                                             const PersistenceDecisionContext& context) {
    DWORD priority_score = SmartQueueToolbox::CalculatePriorityScore(message, config_, context);
    // Convert DWORD priority score to MessageImportance
    return static_cast<MessageImportance>(priority_score % 4); // Map to LOW, MEDIUM, HIGH, CRITICAL
}

DWORD SmartMessageQueue::calculateCurrentTimestamp() const {
    return SmartQueueToolbox::GetCurrentTimestamp();
}

void SmartMessageQueue::maintainQueueOrder() {
    // Sort queue by priority using ToolBox (convert deque to vector)
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());
    SmartQueueToolbox::SortByPriority(queue_vector);

    // Convert back to deque
    message_queue_.assign(queue_vector.begin(), queue_vector.end());
}

// =====================================================================================
// QUEUE MANAGEMENT
// =====================================================================================

bool SmartMessageQueue::configure(const SmartQueueConfiguration& config) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Validate configuration using ToolBox
    if (!SmartQueueToolbox::ValidateQueueConfiguration(config)) {
        return false;
    }

    // Update configuration
    config_ = config;

    // Update ToolBox configuration
    SmartQueueToolbox::SetQueueConfiguration(queue_id_, config_);

    return true;
}

// =====================================================================================
// MESSAGE OPERATIONS
// =====================================================================================

bool SmartMessageQueue::enqueue(const LogMessageData& message, const PersistenceDecisionContext& context) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Check if we can accept the message
    if (!canAcceptMessage(message, context)) {
        return false;
    }

    // Calculate priority score using ToolBox
    DWORD priority_score = SmartQueueToolbox::CalculatePriorityScore(message, config_, context);

    // Convert priority score to MessageImportance for space tracking
    MessageImportance importance = static_cast<MessageImportance>(priority_score % 4);

    // Create queue entry using ToolBox
    QueueEntryMetadata metadata = SmartQueueToolbox::CreateQueueEntry(message, priority_score);

    // Add to queue
    message_queue_.push_back({message, metadata});

    // Update space usage
    updateSpaceUsage(message, importance, true);

    // Maintain queue order
    maintainQueueOrder();

    // Update statistics
    updateQueueStatistics("QUEUE", 1, calculateMessageSpace(message));

    // Notify waiting threads
    queue_condition_.notify_one();

    return true;
}

bool SmartMessageQueue::enqueue(const LogMessageData& message) {
    // Use default context for persistence decision
    PersistenceDecisionContext default_context;
    default_context.current_system_load = 0;
    default_context.current_error_rate = 0;
    default_context.is_emergency_mode = false;

    return enqueue(message, default_context);
}

bool SmartMessageQueue::dequeue(LogMessageData& message, QueueEntryMetadata& metadata) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (message_queue_.empty()) {
        return false;
    }

    // Get next message (highest priority)
    auto& front = message_queue_.front();
    message = front.first;
    metadata = front.second;

    // Remove from queue
    message_queue_.pop_front();

    // Update space usage
    MessageImportance importance = static_cast<MessageImportance>(metadata.priority_score % 10);
    updateSpaceUsage(message, importance, false);

    // Update statistics
    updateQueueStatistics("DEQUEUE", 1, calculateMessageSpace(message));

    return true;
}

bool SmartMessageQueue::peek(LogMessageData& message, QueueEntryMetadata& metadata) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (message_queue_.empty()) {
        return false;
    }

    // Peek at next message without removing
    auto& front = message_queue_.front();
    message = front.first;
    metadata = front.second;

    return true;
}

bool SmartMessageQueue::dequeueWithTimeout(LogMessageData& message, QueueEntryMetadata& metadata, DWORD timeout_ms) {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // Wait for messages if queue is empty
    if (message_queue_.empty()) {
        queue_condition_.wait_for(lock, std::chrono::milliseconds(timeout_ms));
    }

    if (message_queue_.empty()) {
        return false; // Timeout
    }

    // Get next message
    auto& front = message_queue_.front();
    message = front.first;
    metadata = front.second;

    // Remove from queue
    message_queue_.pop_front();

    // Update space usage
    MessageImportance importance = static_cast<MessageImportance>(metadata.priority_score % 10);
    updateSpaceUsage(message, importance, false);

    // Update statistics
    updateQueueStatistics("DEQUEUE", 1, calculateMessageSpace(message));

    return true;
}

// =====================================================================================
// BATCH OPERATIONS
// =====================================================================================

size_t SmartMessageQueue::dequeueBatch(std::vector<LogMessageData>& messages, size_t max_count) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    size_t dequeued_count = 0;
    messages.clear();

    while (dequeued_count < max_count && !message_queue_.empty()) {
        auto& front = message_queue_.front();
        messages.push_back(front.first);

        // Update space usage
        MessageImportance importance = static_cast<MessageImportance>(front.second.priority_score % 10);
        updateSpaceUsage(front.first, importance, false);

        // Remove from queue
        message_queue_.pop_front();
        dequeued_count++;
    }

    // Update statistics
    if (dequeued_count > 0) {
        size_t total_bytes = std::accumulate(messages.begin(), messages.end(), 0UL,
                                           [this](size_t sum, const LogMessageData& msg) {
                                               return sum + calculateMessageSpace(msg);
                                           });
        updateQueueStatistics("DEQUEUE", dequeued_count, total_bytes);
    }

    return dequeued_count;
}

size_t SmartMessageQueue::dequeueForPersistence(std::vector<LogMessageData>& messages,
                                               size_t max_count,
                                               const PersistenceDecisionContext& context) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    size_t dequeued_count = 0;
    messages.clear();

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Create optimal batch using ToolBox
    QueueBatch batch = SmartQueueToolbox::CreateOptimalBatch(queue_vector, config_, max_count);

    // Process batch - use original queue for processing
    size_t processed_count = 0;
    for (const auto& batch_entry : queue_vector) {
        if (dequeued_count >= max_count) break;

        messages.push_back(batch_entry.first);

        // Update space usage
        MessageImportance importance = static_cast<MessageImportance>(batch_entry.second.priority_score % 10);
        updateSpaceUsage(batch_entry.first, importance, false);

        dequeued_count++;
    }

    // Remove processed messages from queue
    for (const auto& processed_message : messages) {
        message_queue_.erase(
            std::remove_if(message_queue_.begin(), message_queue_.end(),
                          [&processed_message](const auto& queue_entry) {
                              return SmartQueueToolbox::CompareMessages(queue_entry.first, processed_message);
                          }),
            message_queue_.end());
    }

    // Update statistics
    if (dequeued_count > 0) {
        size_t total_bytes = std::accumulate(messages.begin(), messages.end(), 0UL,
                                           [this](size_t sum, const LogMessageData& msg) {
                                               return sum + calculateMessageSpace(msg);
                                           });
        updateQueueStatistics("DEQUEUE", dequeued_count, total_bytes);
    }

    return dequeued_count;
}

QueueBatch SmartMessageQueue::createOptimalBatch(size_t max_batch_size) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Use ToolBox to create optimal batch
    return SmartQueueToolbox::CreateOptimalBatch(queue_vector, config_, max_batch_size);
}

QueueBatch SmartMessageQueue::createPriorityBatch() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Use ToolBox to create priority batch
    return SmartQueueToolbox::CreatePriorityBatch(queue_vector, config_);
}

// =====================================================================================
// PRIORITY PRESERVATION
// =====================================================================================

size_t SmartMessageQueue::preservePriorityMessages(size_t count, DWORD duration_ms) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    std::lock_guard<std::mutex> preserve_lock(preservation_mutex_);

    DWORD current_time = calculateCurrentTimestamp();

    // Find messages to preserve
    std::vector<LogMessageData> messages_to_preserve;
    size_t preserved_count = 0;

    for (const auto& entry : message_queue_) {
        if (preserved_count >= count) break;

        // Check if message should be preserved
        if (SmartQueueToolbox::ShouldPreserveMessage(entry.second, current_time, config_)) {
            messages_to_preserve.push_back(entry.first);
            preserved_count++;
        }
    }

    // Mark messages for preservation using ToolBox
    auto updated_metadata = SmartQueueToolbox::MarkForPriorityPreservation(messages_to_preserve, duration_ms);

    // Update queue entries with preservation metadata
    size_t update_count = 0;
    for (auto& entry : message_queue_) {
        for (const auto& updated : updated_metadata) {
            if (entry.first.message_id == messages_to_preserve[update_count].message_id) {
                entry.second = updated;
                update_count++;
                break;
            }
        }
        if (update_count >= updated_metadata.size()) break;
    }

    return preserved_count;
}

size_t SmartMessageQueue::extendPriorityPreservation(DWORD additional_duration_ms) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    std::lock_guard<std::mutex> preserve_lock(preservation_mutex_);

    DWORD current_time = calculateCurrentTimestamp();

    // Find preserved messages that need extension
    std::vector<QueueEntryMetadata> entries_to_extend;

    for (const auto& entry : message_queue_) {
        if (SmartQueueToolbox::IsPriorityPreservationExpired(entry.second, current_time)) {
            entries_to_extend.push_back(entry.second);
        }
    }

    // Extend preservation using ToolBox
    auto extended_entries = SmartQueueToolbox::ExtendPriorityPreservation(entries_to_extend, additional_duration_ms);

    // Update queue entries
    size_t extended_count = 0;
    for (auto& entry : message_queue_) {
        for (const auto& extended : extended_entries) {
            if (entry.second.preservation_expiry == extended.preservation_expiry) {
                entry.second = extended;
                extended_count++;
                break;
            }
        }
    }

    return extended_count;
}

size_t SmartMessageQueue::clearPreservedMessages() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    std::lock_guard<std::mutex> preserve_lock(preservation_mutex_);

    DWORD current_time = calculateCurrentTimestamp();
    size_t cleared_count = 0;

    // Remove expired preserved messages
    message_queue_.erase(
        std::remove_if(message_queue_.begin(), message_queue_.end(),
                      [this, current_time, &cleared_count](auto& entry) {
                          if (SmartQueueToolbox::IsPriorityPreservationExpired(entry.second, current_time)) {
                              // Update space usage
                              MessageImportance importance = static_cast<MessageImportance>(entry.second.priority_score % 10);
                              updateSpaceUsage(entry.first, importance, false);
                              cleared_count++;
                              return true;
                          }
                          return false;
                      }),
        message_queue_.end());

    // Update statistics
    if (cleared_count > 0) {
        updateQueueStatistics("EVICT", cleared_count, 0); // Space already updated above
    }

    return cleared_count;
}

// =====================================================================================
// EVICTION MANAGEMENT
// =====================================================================================

size_t SmartMessageQueue::performIntelligentEviction(size_t space_needed_bytes) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Find messages to evict using ToolBox
    auto messages_to_evict = SmartQueueToolbox::FindMessagesToEvict(queue_vector, space_needed_bytes, config_);

    if (messages_to_evict.empty()) {
        return 0;
    }

    // Execute eviction using ToolBox
    auto eviction_decisions = SmartQueueToolbox::ExecuteEviction(messages_to_evict, "INTELLIGENT");

    // Remove evicted messages from queue
    size_t evicted_count = 0;
    size_t freed_bytes = 0;

    for (const auto& decision : eviction_decisions) {
        // Find and remove message from queue
        auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
                              [&decision](const auto& entry) {
                                  return entry.first.message_id == decision.message_id;
                              });

        if (it != message_queue_.end()) {
            // Update space usage
            MessageImportance importance = static_cast<MessageImportance>(it->second.priority_score % 10);
            updateSpaceUsage(it->first, importance, false);
            freed_bytes += calculateMessageSpace(it->first);

            // Remove from queue
            message_queue_.erase(it);
            evicted_count++;
        }
    }

    // Update statistics
    updateQueueStatistics("EVICT", evicted_count, freed_bytes);

    return freed_bytes;
}

size_t SmartMessageQueue::performAgeBasedEviction(DWORD max_age_seconds) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    DWORD current_time = calculateCurrentTimestamp();

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Find oldest messages using ToolBox
    auto oldest_messages = SmartQueueToolbox::FindOldestMessages(queue_vector, SIZE_MAX, current_time);

    size_t evicted_count = 0;
    size_t freed_bytes = 0;

    for (const auto& old_message : oldest_messages) {
        DWORD message_age = SmartQueueToolbox::CalculateMessageAge(old_message.second, current_time);

        if (message_age > max_age_seconds) {
            // Check if message is preserved
            if (!SmartQueueToolbox::ShouldPreserveMessage(old_message.second, current_time, config_)) {
                // Update space usage
                MessageImportance importance = static_cast<MessageImportance>(old_message.second.priority_score % 10);
                updateSpaceUsage(old_message.first, importance, false);
                freed_bytes += calculateMessageSpace(old_message.first);

                // Remove from queue
                message_queue_.erase(
                    std::remove_if(message_queue_.begin(), message_queue_.end(),
                                  [&old_message](const auto& entry) {
                                      return entry.first.message_id == old_message.first.message_id;
                                  }),
                    message_queue_.end());

                evicted_count++;
            }
        } else {
            break; // Messages are sorted by age, so we can stop here
        }
    }

    // Update statistics
    if (evicted_count > 0) {
        updateQueueStatistics("EVICT", evicted_count, freed_bytes);
    }

    return evicted_count;
}

size_t SmartMessageQueue::performImportanceBasedEviction(MessageImportance min_importance) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    DWORD current_time = calculateCurrentTimestamp();

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Find low priority messages using ToolBox
    auto low_priority_messages = SmartQueueToolbox::FindLowestPriorityMessages(queue_vector, SIZE_MAX);

    size_t evicted_count = 0;
    size_t freed_bytes = 0;

    for (const auto& low_priority_message : low_priority_messages) {
        MessageImportance importance = static_cast<MessageImportance>(low_priority_message.second.priority_score % 10);

        if (importance < min_importance) {
            // Check if message is preserved
            if (!SmartQueueToolbox::ShouldPreserveMessage(low_priority_message.second, current_time, config_)) {
                // Update space usage
                updateSpaceUsage(low_priority_message.first, importance, false);
                freed_bytes += calculateMessageSpace(low_priority_message.first);

                // Remove from queue
                message_queue_.erase(
                    std::remove_if(message_queue_.begin(), message_queue_.end(),
                                  [&low_priority_message](const auto& entry) {
                                      return entry.first.message_id == low_priority_message.first.message_id;
                                  }),
                    message_queue_.end());

                evicted_count++;
            }
        } else {
            break; // Messages are sorted by priority, so we can stop here
        }
    }

    // Update statistics
    if (evicted_count > 0) {
        updateQueueStatistics("EVICT", evicted_count, freed_bytes);
    }

    return evicted_count;
}

size_t SmartMessageQueue::forceEvictionToSize(size_t target_size) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (message_queue_.size() <= target_size) {
        return 0;
    }

    size_t messages_to_remove = message_queue_.size() - target_size;
    size_t evicted_count = 0;
    size_t freed_bytes = 0;

    // Remove oldest non-preserved messages
    DWORD current_time = calculateCurrentTimestamp();

    auto it = message_queue_.begin();
    while (it != message_queue_.end() && evicted_count < messages_to_remove) {
        if (!SmartQueueToolbox::ShouldPreserveMessage(it->second, current_time, config_)) {
            // Update space usage
            MessageImportance importance = static_cast<MessageImportance>(it->second.priority_score % 10);
            updateSpaceUsage(it->first, importance, false);
            freed_bytes += calculateMessageSpace(it->first);

            // Remove message
            it = message_queue_.erase(it);
            evicted_count++;
        } else {
            ++it;
        }
    }

    // Update statistics
    if (evicted_count > 0) {
        updateQueueStatistics("EVICT", evicted_count, freed_bytes);
    }

    return evicted_count;
}

std::vector<std::pair<LogMessageData, QueueEntryMetadata>> SmartMessageQueue::getEvictionCandidates(size_t count) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    DWORD current_time = calculateCurrentTimestamp();

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    // Find oldest non-preserved messages as eviction candidates
    return SmartQueueToolbox::FindOldestMessages(queue_vector, count, current_time);
}

// =====================================================================================
// QUEUE INSPECTION
// =====================================================================================

std::vector<LogMessageData> SmartMessageQueue::getMessagesByImportance(MessageImportance importance) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    std::vector<LogMessageData> result;

    for (const auto& entry : message_queue_) {
        MessageImportance msg_importance = static_cast<MessageImportance>(entry.second.priority_score % 10);
        if (msg_importance == importance) {
            result.push_back(entry.first);
        }
    }

    return result;
}

bool SmartMessageQueue::getOldestMessage(LogMessageData& message, QueueEntryMetadata& metadata) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (message_queue_.empty()) {
        return false;
    }

    // Find oldest message using ToolBox
    DWORD current_time = calculateCurrentTimestamp();

    // Convert deque to vector for ToolBox method
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());

    auto oldest_messages = SmartQueueToolbox::FindOldestMessages(queue_vector, 1, current_time);

    if (!oldest_messages.empty()) {
        message = oldest_messages[0].first;
        metadata = oldest_messages[0].second;
        return true;
    }

    return false;
}

bool SmartMessageQueue::getHighestPriorityMessage(LogMessageData& message, QueueEntryMetadata& metadata) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (message_queue_.empty()) {
        return false;
    }

    // Front of queue is highest priority (queue is sorted by priority)
    message = message_queue_.front().first;
    metadata = message_queue_.front().second;

    return true;
}

// =====================================================================================
// STATISTICS AND MONITORING
// =====================================================================================

SmartQueueStatistics SmartMessageQueue::getStatistics() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Get statistics from ToolBox
    return SmartQueueToolbox::GetQueueStatistics(queue_id_);
}

void SmartMessageQueue::resetStatistics() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Reset ToolBox statistics
    SmartQueueToolbox::ResetQueueStatistics(queue_id_);

    // Reset atomic counters
    total_messages_queued_ = 0;
    total_messages_dequeued_ = 0;
    total_messages_evicted_ = 0;
    current_memory_usage_ = 0;
    last_batch_time_ = 0;
}

std::vector<std::pair<std::string, double>> SmartMessageQueue::getPerformanceMetrics() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Get efficiency metrics from ToolBox
    return SmartQueueToolbox::CalculateEfficiencyMetrics(queue_id_);
}

DWORD SmartMessageQueue::getAverageQueueLatency() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    auto stats = SmartQueueToolbox::GetQueueStatistics(queue_id_);

    if (stats.total_messages_dequeued == 0) {
        return 0;
    }

    return stats.average_processing_time_ms;
}

// =====================================================================================
// QUEUE MAINTENANCE
// =====================================================================================

size_t SmartMessageQueue::cleanupExpiredMessages() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    DWORD current_time = calculateCurrentTimestamp();
    size_t cleaned_count = 0;
    size_t freed_bytes = 0;

    // Remove expired messages
    message_queue_.erase(
        std::remove_if(message_queue_.begin(), message_queue_.end(),
                      [this, current_time, &cleaned_count, &freed_bytes](auto& entry) {
                          if (SmartQueueToolbox::IsQueueEntryExpired(entry.second, current_time)) {
                              // Update space usage
                              MessageImportance importance = static_cast<MessageImportance>(entry.second.priority_score % 10);
                              updateSpaceUsage(entry.first, importance, false);
                              freed_bytes += calculateMessageSpace(entry.first);
                              cleaned_count++;
                              return true;
                          }
                          return false;
                      }),
        message_queue_.end());

    // Update statistics
    if (cleaned_count > 0) {
        updateQueueStatistics("EVICT", cleaned_count, freed_bytes);
    }

    return cleaned_count;
}

bool SmartMessageQueue::compact() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Compaction is implicit in our deque-based implementation
    // We could defragment if needed, but for now just return success
    return true;
}

size_t SmartMessageQueue::clear() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    size_t cleared_count = message_queue_.size();
    size_t freed_bytes = current_memory_usage_.load();

    // Clear queue
    message_queue_.clear();

    // Reset space usage
    for (auto& usage : space_usage_by_importance_) {
        usage.second = 0;
    }

    // Reset memory usage
    current_memory_usage_ = 0;

    // Update statistics
    updateQueueStatistics("EVICT", cleared_count, freed_bytes);

    return cleared_count;
}

bool SmartMessageQueue::resize(size_t new_max_size) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Update configuration
    config_.max_total_size = new_max_size;

    // Update ToolBox configuration
    SmartQueueToolbox::SetQueueConfiguration(queue_id_, config_);

    // If new size is smaller, may need to evict messages
    if (message_queue_.size() > new_max_size) {
        forceEvictionToSize(new_max_size);
    }

    return true;
}

bool SmartMessageQueue::validate() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Validate configuration using ToolBox
    if (!SmartQueueToolbox::ValidateQueueConfiguration(config_)) {
        return false;
    }

    // Validate queue state
    if (message_queue_.size() > config_.max_total_size) {
        return false;
    }

    // Validate space usage tracking
    size_t calculated_memory = 0;
    for (const auto& entry : message_queue_) {
        calculated_memory += calculateMessageSpace(entry.first);
    }

    if (calculated_memory != current_memory_usage_.load()) {
        return false;
    }

    return true;
}

// =====================================================================================
// ADVANCED OPERATIONS
// =====================================================================================

bool SmartMessageQueue::reorderByPriority() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Sort queue by priority using ToolBox (convert deque to vector)
    std::vector<std::pair<LogMessageData, QueueEntryMetadata>> queue_vector;
    queue_vector.assign(message_queue_.begin(), message_queue_.end());
    SmartQueueToolbox::SortByPriority(queue_vector);

    // Convert back to deque
    message_queue_.assign(queue_vector.begin(), queue_vector.end());

    return true;
}

bool SmartMessageQueue::moveToFront(uint32_t message_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Find message by ID
    auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
                          [message_id](const auto& entry) {
                              return entry.first.message_id == message_id;
                          });

    if (it == message_queue_.end()) {
        return false;
    }

    // Move to front
    std::rotate(message_queue_.begin(), it, it + 1);

    return true;
}

bool SmartMessageQueue::moveToBack(uint32_t message_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Find message by ID
    auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
                          [message_id](const auto& entry) {
                              return entry.first.message_id == message_id;
                          });

    if (it == message_queue_.end()) {
        return false;
    }

    // Move to back
    std::rotate(it, it + 1, message_queue_.end());

    return true;
}

bool SmartMessageQueue::findMessage(uint32_t message_id, LogMessageData& message, QueueEntryMetadata& metadata) const {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Find message by ID
    auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
                          [message_id](const auto& entry) {
                              return entry.first.message_id == message_id;
                          });

    if (it == message_queue_.end()) {
        return false;
    }

    message = it->first;
    metadata = it->second;

    return true;
}

bool SmartMessageQueue::removeMessage(uint32_t message_id) {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    // Find and remove message by ID
    auto it = std::find_if(message_queue_.begin(), message_queue_.end(),
                          [message_id](const auto& entry) {
                              return entry.first.message_id == message_id;
                          });

    if (it == message_queue_.end()) {
        return false;
    }

    // Update space usage
    MessageImportance importance = static_cast<MessageImportance>(it->second.priority_score % 10);
    updateSpaceUsage(it->first, importance, false);

    // Remove message
    message_queue_.erase(it);

    // Update statistics
    updateQueueStatistics("EVICT", 1, calculateMessageSpace(it->first));

    return true;
}

// =====================================================================================
// SYNCHRONIZATION
// =====================================================================================

bool SmartMessageQueue::waitForMessages(DWORD timeout_ms) {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    if (message_queue_.empty()) {
        queue_condition_.wait_for(lock, std::chrono::milliseconds(timeout_ms));
    }

    return !message_queue_.empty();
}

void SmartMessageQueue::notifyMessageAvailable() {
    queue_condition_.notify_one();
}

// =====================================================================================
// NON-MEMBER FUNCTIONS
// =====================================================================================

SmartMessageQueue CreateApplicationSmartQueue(const std::string& application_name,
                                             const std::string& queue_name,
                                             size_t max_size) {
    return SmartMessageQueue(queue_name, application_name, max_size);
}

std::ostream& operator<<(std::ostream& os, const SmartMessageQueue& queue) {
    os << "SmartMessageQueue[id: " << queue.getId()
       << ", size: " << queue.size()
       << ", memory: " << queue.getMemoryUsage() << " bytes"
       << ", queued: " << queue.getTotalQueued()
       << ", dequeued: " << queue.getTotalDequeued()
       << ", evicted: " << queue.getTotalEvicted() << "]";
    return os;
}
