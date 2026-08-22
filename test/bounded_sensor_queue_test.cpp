#include "bounded_sensor_queue.hpp"

#include <cstddef>
#include <iostream>
#include <thread>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "CHECK failed: " #condition << '\n';                       \
      return 1;                                                                \
    }                                                                          \
  } while (false)

int main() {
  fast_livo::BoundedSensorQueue<int, 200U> imu_queue;
  fast_livo::BoundedSensorQueue<int, 10U> lidar_queue;

  // estimator 是否忙只影响 drain；独立 callback producer 仍可完整接收
  // 1 s 内 200 Hz IMU 和 10 Hz LiDAR。
  bool callback_ok = true;
  std::thread callback_thread([&imu_queue, &lidar_queue, &callback_ok]() {
    for (int sequence = 0; sequence < 200; ++sequence) {
      if (imu_queue.push(sequence)) callback_ok = false;
      if (sequence % 20 == 0 && lidar_queue.push(sequence / 20))
        callback_ok = false;
    }
  });
  callback_thread.join();
  CHECK(callback_ok);

  const auto continuous = imu_queue.drain();
  CHECK(!continuous.continuity_lost);
  CHECK(continuous.items.size() == 200U);
  for (std::size_t index = 0; index < continuous.items.size(); ++index) {
    CHECK(continuous.items[index] == static_cast<int>(index));
  }
  const auto lidar_continuous = lidar_queue.drain();
  CHECK(!lidar_continuous.continuity_lost);
  CHECK(lidar_continuous.items.size() == 10U);

  for (int sequence = 0; sequence < 201; ++sequence) {
    imu_queue.push(sequence);
  }
  const auto overflowed = imu_queue.drain();
  CHECK(overflowed.continuity_lost);
  CHECK(overflowed.items.size() == 200U);
  CHECK(overflowed.items.front() == 1);
  CHECK(overflowed.items.back() == 200);
  CHECK(imu_queue.overflowTotal() == 1U);
  return 0;
}
