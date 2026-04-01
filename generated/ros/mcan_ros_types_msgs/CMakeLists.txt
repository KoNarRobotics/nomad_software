cmake_minimum_required(VERSION 3.8)
project(mcan_ros_types_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

# Note: This package has no .msg files (all types use ros_mapping)
# Package provides conversion utilities only


# Create interface library for C++ type headers
add_library(mcan_ros_types_cpp_headers INTERFACE)
target_include_directories(mcan_ros_types_cpp_headers INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(mcan_ros_types_cpp_headers INTERFACE sensor_msgs
 geometry_msgs
)
install(TARGETS mcan_ros_types_cpp_headers
  EXPORT mcan_ros_types_cpp_headersTargets)
install(EXPORT mcan_ros_types_cpp_headersTargets
  DESTINATION share/${PROJECT_NAME}/cmake)

find_package(rclcpp REQUIRED)

install(
DIRECTORY include/
DESTINATION include
)
# Export include directories for downstream packages
ament_export_include_directories(include)
ament_export_targets(mcan_ros_types_cpp_headersTargets)
ament_export_dependencies(std_msgs)

ament_package()