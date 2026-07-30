// Copyright 2026 Nomad Robotics
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "suspension_driver/suspension_driver_node.hpp"

#include <algorithm>
#include <chrono>
#include <functional>

namespace suspension_driver {

using std::placeholders::_1;

SuspensionDriverNode::SuspensionDriverNode(const rclcpp::NodeOptions &options)
: rclcpp::Node("suspension_driver", options) {
  // --- Parameters ----------------------------------------------------------
  RobotGeometry geometry;
  geometry.wheel_radius = declare_parameter<double>("wheel_radius", 0.1);
  geometry.wheel_base   = declare_parameter<double>("wheel_base", 0.5);
  geometry.track_width  = declare_parameter<double>("track_width", 0.5);

  const double control_rate_hz = declare_parameter<double>("control_rate", 50.0);
  max_drive_velocity_          = declare_parameter<double>("max_drive_velocity", 0.0);
  cmd_vel_timeout_s_           = declare_parameter<double>("cmd_vel_timeout", 0.5);
  publish_tf_                  = declare_parameter<bool>("publish_tf", true);
  odom_frame_id_               = declare_parameter<std::string>("odom_frame_id", "odom");
  base_frame_id_               = declare_parameter<std::string>("base_frame_id", "base_link");

  kinematics_ = std::make_unique<SwerveKinematics>(geometry);

  // --- Subscribers -----------------------------------------------------------
  // Commands use best-effort-tolerant depth-1 QoS semantics via SensorDataQoS
  // for high-rate feedback, and a small reliable queue for /cmd_vel.
  cmd_vel_sub_ =
  create_subscription<geometry_msgs::msg::Twist>("/cmd_vel", rclcpp::QoS(1),
                                                 std::bind(&SuspensionDriverNode::cmd_vel_callback, this, _1));

  wheel_velocity_sub_ =
  create_subscription<std_msgs::msg::Float64MultiArray>("/wheel_velocity", rclcpp::SensorDataQoS(),
                                                        std::bind(&SuspensionDriverNode::wheel_velocity_callback, this, _1));

  wheel_position_sub_ = create_subscription<std_msgs::msg::Float64MultiArray>(
  "/wheel_pos", rclcpp::SensorDataQoS(), std::bind(&SuspensionDriverNode::wheel_position_callback, this, _1));

  steering_position_sub_ =
  create_subscription<std_msgs::msg::Float64MultiArray>("/wheel_rotation_pos", rclcpp::SensorDataQoS(),
                                                        std::bind(&SuspensionDriverNode::steering_position_callback,
                                                                  this, _1));

  RCLCPP_INFO(get_logger(),
              "suspension_driver up: wheel_radius=%.3f m, wheel_base=%.3f m, track_width=%.3f m, "
              "control_rate=%.1f Hz",
              geometry.wheel_radius, geometry.wheel_base, geometry.track_width, control_rate_hz);
}

std::optional<std::array<double, kNumWheels>>
SuspensionDriverNode::to_wheel_array(const std_msgs::msg::Float64MultiArray &msg, const char *topic_name) {
  if(msg.data.size() != kNumWheels) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000, "Ignoring %s message with %zu values (expected %zu)",
                         topic_name, msg.data.size(), kNumWheels);
    return std::nullopt;
  }
  std::array<double, kNumWheels> values{};
  std::copy_n(msg.data.begin(), kNumWheels, values.begin());
  return values;
}

void SuspensionDriverNode::cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
  const std::lock_guard<std::mutex> lock(mutex_);
  target_twist_.vx    = msg->linear.x;
  target_twist_.vy    = msg->linear.y;
  target_twist_.omega = msg->angular.z;
  last_cmd_time_      = now();
}

void SuspensionDriverNode::wheel_velocity_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg) {
  const auto values = to_wheel_array(*msg, "/wheel_velocity");
  if(!values) {
    return;
  }
  const std::lock_guard<std::mutex> lock(mutex_);
  wheel_states_.drive_velocities = *values;
}

void SuspensionDriverNode::wheel_position_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg) {
  const auto values = to_wheel_array(*msg, "/wheel_pos");
  if(!values) {
    return;
  }
  const std::lock_guard<std::mutex> lock(mutex_);
  wheel_positions_rad_ = *values;
}

void SuspensionDriverNode::steering_position_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg) {
  const auto values = to_wheel_array(*msg, "/wheel_rotation_pos");
  if(!values) {
    return;
  }
  const std::lock_guard<std::mutex> lock(mutex_);
  wheel_states_.steering_angles = *values;
  steering_feedback_received_   = true;
}

} // namespace suspension_driver

/// Entry point: spins the driver on a single-threaded executor. All shared
/// state is additionally mutex-guarded, so the node stays correct if it is
/// ever moved onto a multi-threaded executor or composed with other nodes.
int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<suspension_driver::SuspensionDriverNode>());
  rclcpp::shutdown();
  return 0;
}
