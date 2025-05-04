cmake_minimum_required(VERSION 3.8)
project(shared_types_nomad)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 20)
# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)


add_library(${PROJECT_NAME} 
 src/shared_types_nomad.cpp
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  include
)

ament_target_dependencies(
  ${PROJECT_NAME}
  rclcpp
)


install(TARGETS 
  ${PROJECT_NAME} 
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(TARGETS shared_types_nomad
  EXPORT export_shared_types_nomad
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(rclcpp)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
