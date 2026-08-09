file(READ "${PROJECT_SOURCE_DIR}/src/LIVMapper.cpp" mapper_source)

foreach(required_text
    "pubOdomAftMapped->publish(odomAftMapped);\n  recordPublishedMessage(odometry_published_count);"
    "pubLaserCloudFullRes->publish(laserCloudmsg);\n  recordPublishedMessage(cloud_published_count);"
    "printPublishRateSummary();\n  savePCD();"
    "ODOMETRY_PUBLISHED_COUNT="
    "CLOUD_PUBLISHED_COUNT="
    "RATE_WINDOW_START="
    "RATE_WINDOW_DURATION="
    "FAST_LIVO2_PRODUCER_ODOMETRY_RATE_HZ="
    "FAST_LIVO2_PRODUCER_CLOUD_RATE_HZ=")
  string(FIND "${mapper_source}" "${required_text}" match_index)
  if(match_index EQUAL -1)
    message(FATAL_ERROR "Missing producer rate evidence contract: ${required_text}")
  endif()
endforeach()
