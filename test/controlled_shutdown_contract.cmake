file(READ "${PROJECT_SOURCE_DIR}/include/LIVMapper.h" mapper_header)
file(READ "${PROJECT_SOURCE_DIR}/src/LIVMapper.cpp" mapper_source)
file(READ "${PROJECT_SOURCE_DIR}/src/main.cpp" main_source)

foreach(required_text
    "std::shared_ptr<tf2_ros::TransformBroadcaster> transform_broadcaster;"
    "transform_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this->node);"
    "catch (const rclcpp::exceptions::RCLError &error)"
    "if (rclcpp::ok()) throw;"
    "FAST_LIVO2_CONTROLLED_SHUTDOWN=")
  string(FIND "${mapper_header}\n${mapper_source}" "${required_text}" match_index)
  if(match_index EQUAL -1)
    message(FATAL_ERROR "Missing controlled shutdown contract: ${required_text}")
  endif()
endforeach()

foreach(forbidden_text
    "static std::shared_ptr<tf2_ros::TransformBroadcaster> br;"
    "br = std::make_shared<tf2_ros::TransformBroadcaster>(this->node);")
  string(FIND "${mapper_source}" "${forbidden_text}" match_index)
  if(NOT match_index EQUAL -1)
    message(FATAL_ERROR "Per-message broadcaster creation remains: ${forbidden_text}")
  endif()
endforeach()

string(FIND "${main_source}" "if (rclcpp::ok()) rclcpp::shutdown();" guarded_shutdown)
if(guarded_shutdown EQUAL -1)
  message(FATAL_ERROR "main must not repeat shutdown after the signal handler")
endif()
