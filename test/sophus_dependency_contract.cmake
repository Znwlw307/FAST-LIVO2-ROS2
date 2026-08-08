file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" cmake_source)

string(REGEX MATCH
  "set\\(COMMON_DEPENDENCIES[^\\)]*Sophus::Sophus[^\\)]*\\)"
  sophus_common_dependency
  "${cmake_source}"
)
if(sophus_common_dependency STREQUAL "")
  message(FATAL_ERROR "COMMON_DEPENDENCIES must contain Sophus::Sophus")
endif()

set(pre_link_contract [=[target_link_libraries(pre ${COMMON_DEPENDENCIES})]=])
string(FIND "${cmake_source}" "${pre_link_contract}" pre_link_index)
if(pre_link_index EQUAL -1)
  message(FATAL_ERROR "The pre target must consume COMMON_DEPENDENCIES")
endif()

set(legacy_sophus_include [=[${Sophus_INCLUDE_DIRS}]=])
set(legacy_sophus_libraries [=[${Sophus_LIBRARIES}]=])
foreach(legacy_reference IN ITEMS "${legacy_sophus_include}" "${legacy_sophus_libraries}")
  string(FIND "${cmake_source}" "${legacy_reference}" legacy_reference_index)
  if(NOT legacy_reference_index EQUAL -1)
    message(FATAL_ERROR "Legacy Sophus variable must not be used: ${legacy_reference}")
  endif()
endforeach()
