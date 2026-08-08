file(READ "${PROJECT_SOURCE_DIR}/src/LIVMapper.cpp" mapper_source)
file(READ "${PROJECT_SOURCE_DIR}/src/IMU_Processing.cpp" imu_source)
file(READ "${PROJECT_SOURCE_DIR}/src/preprocess.cpp" preprocess_source)

foreach(required_text
    "kLidarSubscriptionDepth = 10"
    "kImuSubscriptionDepth = 200"
    "lid_topic, kLidarSubscriptionDepth"
    "imu_topic, kImuSubscriptionDepth"
    "RCLCPP_WARN_THROTTLE"
    "kRuntimeLogThrottleMs = 1000")
  string(FIND "${mapper_source}" "${required_text}" match_index)
  if(match_index EQUAL -1)
    message(FATAL_ERROR "Missing realtime backlog policy: ${required_text}")
  endif()
endforeach()

foreach(forbidden_text
    "get imu at time:"
    "got imu:"
    "Get LiDAR, its header time:"
    "get point cloud at time:")
  string(FIND "${mapper_source}" "${forbidden_text}" match_index)
  if(NOT match_index EQUAL -1)
    message(FATAL_ERROR "Per-message runtime log is still enabled: ${forbidden_text}")
  endif()
endforeach()

string(FIND "${imu_source}" "#ifdef DEBUG_PRINT" debug_guard_index)
if(debug_guard_index EQUAL -1)
  message(FATAL_ERROR "Per-IMU file logging is not guarded by DEBUG_PRINT")
endif()

foreach(forbidden_text
    "[ Preprocess ] Input point number:"
    "[ Preprocess ] Output point number:"
    "Feature extraction time:")
  string(FIND "${preprocess_source}" "${forbidden_text}" match_index)
  if(NOT match_index EQUAL -1)
    message(FATAL_ERROR "Per-LiDAR preprocessing log is still enabled: ${forbidden_text}")
  endif()
endforeach()
