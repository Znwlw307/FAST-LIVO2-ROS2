#pragma once

/**
 * @file bounded_sensor_queue.hpp
 * @brief 为 FAST-LIVO2 传感器入口提供固定容量、丢最旧项的线程安全队列。
 *
 * 本文件只负责 callback 与 estimator consumer 之间的所有权转移，不执行传感器
 * 预处理、同步、deskew 或 estimator 状态修改。
 */

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace fast_livo {

/** 固定容量的实时输入队列；满队列时保留最近连续窗口并报告 overflow。 */
template <typename T, std::size_t Capacity>
class BoundedSensorQueue {
  static_assert(Capacity > 0U, "sensor queue capacity must be positive");

 public:
  struct DrainResult {
    std::vector<T> items;
    bool continuity_lost{false};
  };

  /**
   * @brief 入队并在满时丢弃最旧项。
   * @return 本次是否发生 overflow。
   * @thread_safety 可与 drain/size 并发调用。
   */
  bool push(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    const bool overflow = queue_.size() == Capacity;
    if (overflow) {
      queue_.pop_front();
      continuity_lost_since_drain_ = true;
      ++overflow_total_;
    }
    queue_.push_back(std::move(item));
    return overflow;
  }

  /** 将当前有界批次移交给唯一 consumer，callback 不等待 consumer 处理。 */
  DrainResult drain() {
    std::lock_guard<std::mutex> lock(mutex_);
    DrainResult result;
    result.items.reserve(queue_.size());
    while (!queue_.empty()) {
      result.items.push_back(std::move(queue_.front()));
      queue_.pop_front();
    }
    result.continuity_lost = continuity_lost_since_drain_;
    continuity_lost_since_drain_ = false;
    return result;
  }

  [[nodiscard]] std::size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  [[nodiscard]] std::uint64_t overflowTotal() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return overflow_total_;
  }

 private:
  mutable std::mutex mutex_;
  std::deque<T> queue_;
  std::uint64_t overflow_total_{0U};
  bool continuity_lost_since_drain_{false};
};

}  // namespace fast_livo
