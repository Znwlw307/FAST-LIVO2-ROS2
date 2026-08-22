#include "imu_coverage_contract.hpp"

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

int main() {
  using fast_livo::ImuCoverageStatus;
  const double none = std::numeric_limits<double>::quiet_NaN();

  std::vector<double> jittered;
  double time = 10.0;
  for (int index = 0; index < 30; ++index) {
    time += index % 2 == 0 ? 0.004 : 0.008;
    jittered.push_back(time);
  }
  CHECK(fast_livo::validateImuCoverage(
            jittered, 10.0, jittered.back(), none, false).status ==
        ImuCoverageStatus::kValid);

  auto missing = jittered;
  for (auto &timestamp : missing) {
    if (timestamp > 10.08) timestamp += 0.5;
  }
  CHECK(fast_livo::validateImuCoverage(
            missing, 10.0, missing.back(), none, false).status ==
        ImuCoverageStatus::kMissingCoverage);

  auto extreme = jittered;
  for (auto &timestamp : extreme) {
    if (timestamp > 10.08) timestamp += 10.7445;
  }
  CHECK(fast_livo::validateImuCoverage(
            extreme, 10.0, extreme.back(), none, false).status ==
        ImuCoverageStatus::kMissingCoverage);
  CHECK(fast_livo::validateImuCoverage(
            jittered, 10.0, jittered.back(), none, true).status ==
        ImuCoverageStatus::kExplicitDiscontinuity);
  return 0;
}
