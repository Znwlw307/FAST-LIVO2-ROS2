#include "voxel_grid_safety.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "CHECK failed: " #condition << '\n';                       \
      return 1;                                                                \
    }                                                                          \
  } while (false)

struct Point { double x; double y; double z; };
struct Cloud { std::vector<Point> points; };

int main() {
  using fast_livo::VoxelGridInputStatus;
  Cloud normal;
  normal.points.reserve(15000U);
  for (int index = 0; index < 15000; ++index) {
    normal.points.push_back(
        {0.01 * (index % 100), 0.01 * ((index / 100) % 100),
         0.01 * (index / 10000)});
  }
  CHECK(fast_livo::validateVoxelGridInput(normal, 0.1) ==
        VoxelGridInputStatus::kSafe);

  Cloud non_finite{{{0.0, 0.0, std::numeric_limits<double>::quiet_NaN()}}};
  CHECK(fast_livo::validateVoxelGridInput(non_finite, 0.1) ==
        VoxelGridInputStatus::kNonFinitePoint);
  non_finite.points[0].z = std::numeric_limits<double>::infinity();
  CHECK(fast_livo::validateVoxelGridInput(non_finite, 0.1) ==
        VoxelGridInputStatus::kNonFinitePoint);

  Cloud huge_coordinate{{{1.0e20, 0.0, 0.0}}};
  CHECK(fast_livo::validateVoxelGridInput(huge_coordinate, 0.1) ==
        VoxelGridInputStatus::kIndexOverflow);
  Cloud huge_extent{{{0.0, 0.0, 0.0}, {50000.0, 50000.0, 1.0}}};
  CHECK(fast_livo::validateVoxelGridInput(huge_extent, 0.1) ==
        VoxelGridInputStatus::kIndexProductOverflow);
  return 0;
}
