cmake_minimum_required(VERSION 3.8)
project(suspension_position_module_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(mc_can_driver REQUIRED)
find_package(mc_plugin_driver REQUIRED)
find_package(mcan_basic_module_msgs REQUIRED)
find_package(mcan_ros_types_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

find_package(rosidl_default_generators REQUIRED)

# Generate messages
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CurrentPositionLeft.msg"
  "msg/CurrentPositionRight.msg"
  "msg/DiscoverDevices.msg"
  "msg/EnterConfigurationMode.msg"
  "msg/FlashIndicatorLed.msg"
  "msg/GetHardwareType.msg"
  "msg/PingModule.msg"
  "msg/SetDeviceNodeId.msg"
  "msg/SetEncoderOffsetLeft.msg"
  "msg/SetEncoderOffsetRight.msg"
  DEPENDENCIES std_msgs geometry_msgs mc_can_driver mc_plugin_driver mcan_basic_module_msgs mcan_ros_types_msgs sensor_msgs
)


# Create interface library for C++ type headers
add_library(suspension_position_module_cpp_headers INTERFACE)
target_include_directories(suspension_position_module_cpp_headers INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(suspension_position_module_cpp_headers INTERFACE mcan_basic_module_msgs
 sensor_msgs
 geometry_msgs
 mcan_ros_types_msgs
)
install(TARGETS suspension_position_module_cpp_headers
  EXPORT suspension_position_module_cpp_headersTargets)
install(EXPORT suspension_position_module_cpp_headersTargets
  DESTINATION share/${PROJECT_NAME}/cmake)

find_package(rclcpp REQUIRED)

# Module plugin library
add_library(suspension_position_module_plugin SHARED  src/suspension_position_module_plugin.cpp)
target_compile_features(suspension_position_module_plugin PUBLIC c_std_99 cxx_std_23)
target_include_directories(suspension_position_module_plugin PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)
target_link_libraries(suspension_position_module_plugin  PUBLIC
    ${pluginlib_TARGETS}
    suspension_position_module_cpp_headers
    yaml-cpp
    mc_plugin_driver::mc_plugin_driver
)

ament_target_dependencies(suspension_position_module_plugin
  PUBLIC
    rclcpp
    pluginlib
    yaml-cpp
    geometry_msgs
    mcan_basic_module_msgs
    mcan_ros_types_msgs
    sensor_msgs

)

rosidl_get_typesupport_target(cpp_typesupport_target  ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(suspension_position_module_plugin PUBLIC ${cpp_typesupport_target})

install(TARGETS suspension_position_module_plugin
  DESTINATION lib/${PROJECT_NAME})

pluginlib_export_plugin_description_file(mc_plugin_driver plugins.xml)
install(TARGETS suspension_position_module_plugin
  DESTINATION lib/${PROJECT_NAME})
install(
DIRECTORY include/
DESTINATION include
)
# Export include directories for downstream packages
ament_export_include_directories(include)
ament_export_targets(suspension_position_module_cpp_headersTargets)
ament_export_dependencies(mc_can_driver mc_plugin_driver mcan_basic_module_msgs mcan_ros_types_msgs std_msgs)

ament_package()