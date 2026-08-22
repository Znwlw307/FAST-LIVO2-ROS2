#pragma once

/**
 * @file voxel_grid_safety.hpp
 * @brief 在调用 PCL VoxelGrid 前校验有限坐标与 int32 voxel index 数学边界。
 *
 * 本文件不限制正常飞行范围；拒绝条件仅来自数值有限性、leaf size 和 PCL 使用的
 * int32 index/product 上限。
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace fast_livo {

enum class VoxelGridInputStatus {
  kSafe,
  kEmpty,
  kInvalidLeafSize,
  kPointCountOverflow,
  kNonFinitePoint,
  kIndexOverflow,
  kIndexProductOverflow,
};

/** 校验任意具有 points 和 x/y/z 字段的 cloud。 */
template <typename Cloud>
VoxelGridInputStatus validateVoxelGridInput(const Cloud &cloud,
                                            const double leaf_size) {
  if (cloud.points.empty()) return VoxelGridInputStatus::kEmpty;
  if (!std::isfinite(leaf_size) || leaf_size <= 0.0)
    return VoxelGridInputStatus::kInvalidLeafSize;
  if (cloud.points.size() >
      static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
    return VoxelGridInputStatus::kPointCountOverflow;

  double minimum[3] = {std::numeric_limits<double>::infinity(),
                       std::numeric_limits<double>::infinity(),
                       std::numeric_limits<double>::infinity()};
  double maximum[3] = {-std::numeric_limits<double>::infinity(),
                       -std::numeric_limits<double>::infinity(),
                       -std::numeric_limits<double>::infinity()};
  for (const auto &point : cloud.points) {
    const double coordinate[3] = {point.x, point.y, point.z};
    for (std::size_t axis = 0; axis < 3U; ++axis) {
      if (!std::isfinite(coordinate[axis]))
        return VoxelGridInputStatus::kNonFinitePoint;
      minimum[axis] = std::min(minimum[axis], coordinate[axis]);
      maximum[axis] = std::max(maximum[axis], coordinate[axis]);
    }
  }

  std::int64_t dimensions[3] = {};
  constexpr double kIndexMinimum =
      static_cast<double>(std::numeric_limits<std::int32_t>::min());
  constexpr double kIndexMaximum =
      static_cast<double>(std::numeric_limits<std::int32_t>::max());
  for (std::size_t axis = 0; axis < 3U; ++axis) {
    const double minimum_index = std::floor(minimum[axis] / leaf_size);
    const double maximum_index = std::floor(maximum[axis] / leaf_size);
    if (!std::isfinite(minimum_index) || !std::isfinite(maximum_index) ||
        minimum_index < kIndexMinimum || maximum_index > kIndexMaximum)
      return VoxelGridInputStatus::kIndexOverflow;
    dimensions[axis] = static_cast<std::int64_t>(maximum_index) -
                       static_cast<std::int64_t>(minimum_index) + 1;
  }

  constexpr std::int64_t kProductMaximum =
      std::numeric_limits<std::int32_t>::max();
  std::int64_t product = 1;
  for (const std::int64_t dimension : dimensions) {
    if (dimension <= 0 || dimension > kProductMaximum / product)
      return VoxelGridInputStatus::kIndexProductOverflow;
    product *= dimension;
  }
  return VoxelGridInputStatus::kSafe;
}

inline const char *voxelGridInputStatusName(
    const VoxelGridInputStatus status) {
  switch (status) {
  case VoxelGridInputStatus::kSafe: return "SAFE";
  case VoxelGridInputStatus::kEmpty: return "EMPTY";
  case VoxelGridInputStatus::kInvalidLeafSize: return "INVALID_LEAF_SIZE";
  case VoxelGridInputStatus::kPointCountOverflow: return "POINT_COUNT_OVERFLOW";
  case VoxelGridInputStatus::kNonFinitePoint: return "NON_FINITE_POINT";
  case VoxelGridInputStatus::kIndexOverflow: return "INDEX_OVERFLOW";
  case VoxelGridInputStatus::kIndexProductOverflow:
    return "INDEX_PRODUCT_OVERFLOW";
  }
  return "UNKNOWN";
}

}  // namespace fast_livo
