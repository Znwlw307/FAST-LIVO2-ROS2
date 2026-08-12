file(READ "${PROJECT_SOURCE_DIR}/include/LIVMapper.h" mapper_header)
file(READ "${PROJECT_SOURCE_DIR}/src/LIVMapper.cpp" mapper_source)

foreach(required_text
    "~/prepare_shutdown"
    "input_admission_open = false;"
    "bool LIVMapper::producerIdle() const"
    "!lio_frame_inflight && pendingLidarFrameCount() == 0"
    "REJECTED_INPUT_AFTER_QUIESCE_COUNT="
    "PENDING_FINAL_PUBLICATION="
    "ROS_CONTEXT_ALIVE="
    "PUBLISHER_ALIVE="
    "if (!beginInputCallback(lidar_en)) return;"
    "if (!beginInputCallback(imu_en)) return;"
    "if (!beginInputCallback(img_en)) return;"
    "while (rclcpp::ok())")
  string(FIND "${mapper_header}\n${mapper_source}" "${required_text}" match_index)
  if(match_index EQUAL -1)
    message(FATAL_ERROR "Missing producer quiescence contract: ${required_text}")
  endif()
endforeach()

string(REGEX MATCH
  "void LIVMapper::requestProducerQuiescence\\([^}]*rclcpp::shutdown"
  premature_shutdown "${mapper_source}")
if(premature_shutdown)
  message(FATAL_ERROR "prepare_shutdown must keep the ROS context and DataWriter alive")
endif()
