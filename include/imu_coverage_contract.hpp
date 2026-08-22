#pragma once

/**
 * @file imu_coverage_contract.hpp
 * @brief 判定单个 FAST-LIVO2 measurement group 的 IMU source-time coverage。
 *
 * 本文件不传播状态，也不选择 localization reset；调用方只据 typed 结果拒绝
 * 当前 measurement，并在后续连续窗口重建时间基准。
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace fast_livo {

enum class ImuCoverageStatus {
  kValid,
  kEmpty,
  kInvalidTimestamp,
  kRegression,
  kMissingCoverage,
  kExplicitDiscontinuity,
};

struct ImuCoverageResult {
  ImuCoverageStatus status{ImuCoverageStatus::kEmpty};
  double maximum_delta_seconds{0.0};
};

/** 使用既有 200 ms 合同区分 bounded jitter 与缺失 coverage。 */
inline ImuCoverageResult validateImuCoverage(
    const std::vector<double> &timestamps, const double required_start,
    const double required_end, const double previous_timestamp,
    const bool explicit_discontinuity,
    const double maximum_gap_seconds = 0.2) {
  if (explicit_discontinuity) {
    return {ImuCoverageStatus::kExplicitDiscontinuity, 0.0};
  }
  if (timestamps.empty()) return {ImuCoverageStatus::kEmpty, 0.0};
  if (!std::isfinite(required_start) || !std::isfinite(required_end) ||
      required_end < required_start || !std::isfinite(maximum_gap_seconds) ||
      maximum_gap_seconds <= 0.0) {
    return {ImuCoverageStatus::kInvalidTimestamp, 0.0};
  }

  double maximum_delta = 0.0;
  double previous = std::isfinite(previous_timestamp)
                        ? previous_timestamp
                        : timestamps.front();
  for (std::size_t index = 0; index < timestamps.size(); ++index) {
    const double timestamp = timestamps[index];
    if (!std::isfinite(timestamp)) {
      return {ImuCoverageStatus::kInvalidTimestamp, maximum_delta};
    }
    if ((index > 0U || std::isfinite(previous_timestamp)) &&
        timestamp <= previous) {
      return {ImuCoverageStatus::kRegression, maximum_delta};
    }
    if (index > 0U || std::isfinite(previous_timestamp)) {
      const double delta = timestamp - previous;
      maximum_delta = std::max(maximum_delta, delta);
      if (delta > maximum_gap_seconds) {
        return {ImuCoverageStatus::kMissingCoverage, maximum_delta};
      }
    }
    previous = timestamp;
  }

  if (timestamps.front() > required_start + maximum_gap_seconds ||
      timestamps.back() < required_end - maximum_gap_seconds) {
    return {ImuCoverageStatus::kMissingCoverage, maximum_delta};
  }
  return {ImuCoverageStatus::kValid, maximum_delta};
}

inline const char *imuCoverageStatusName(const ImuCoverageStatus status) {
  switch (status) {
  case ImuCoverageStatus::kValid: return "VALID";
  case ImuCoverageStatus::kEmpty: return "EMPTY";
  case ImuCoverageStatus::kInvalidTimestamp: return "INVALID_TIMESTAMP";
  case ImuCoverageStatus::kRegression: return "TIMESTAMP_REGRESSION";
  case ImuCoverageStatus::kMissingCoverage: return "MISSING_COVERAGE";
  case ImuCoverageStatus::kExplicitDiscontinuity:
    return "EXPLICIT_DISCONTINUITY";
  }
  return "UNKNOWN";
}

}  // namespace fast_livo
