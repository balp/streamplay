#
#
#

cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10)
pkg_check_modules(GSTREAMER_PLUGINS_BASE REQUIRED gstreamer-plugins-base-0.10)

message(STATUS "GStreamer: ${GSTREAMER_FOUND} ${GSTREAMER_LIBRARIES} " )

add_definitions(-Wall -Wextra -Werror)

include_directories(
  ${GSTREAMER_INCLUDE_DIRS}
)
add_executable(streamplay
  streamplay.cpp
)

target_link_libraries(streamplay
  rt
  ${GSTREAMER_LIBRARIES}
)
