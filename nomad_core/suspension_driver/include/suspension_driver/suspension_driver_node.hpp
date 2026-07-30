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

#ifndef SUSPENSION_DRIVER__SUSPENSION_DRIVER_NODE_HPP_
#define SUSPENSION_DRIVER__SUSPENSION_DRIVER_NODE_HPP_

#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "tf2_ros/transform_broadcaster.h"

#include "suspension_driver/swerve_kinematics.hpp"

namespace suspension_driver {

/// ROS 2 wrapper node around the SwerveKinematics library.
///
/// Subscriptions (wheel arrays are ordered FL, FR, RL, RR):
///   * /cmd_vel            geometry_msgs/msg/Twist        target body twist
///   * /wheel_velocity     std_msgs/msg/Float64MultiArray drive wheel velocities [rad/s]
///   * /wheel_pos          std_msgs/msg/Float64MultiArray drive wheel positions [rad]
///   * /wheel_rotation_pos std_msgs/msg/Float64MultiArray steering angles [rad]
///
/// Publications:
///   * /drive_velocity_cmd    std_msgs/msg/Float64MultiArray 4x wheel velocity targets [rad/s]
///   * /steering_position_cmd std_msgs/msg/Float64MultiArray 4x steering angle targets [rad]
///   * /wheel_odom            nav_msgs/msg/Odometry          integrated wheel odometry
///   * TF odom -> base_link (optional, `publish_tf` parameter)
///
/// A fixed-rate control loop runs the inverse kinematics and publishes the
/// wheel commands; a command watchdog stops the base if /cmd_vel goes stale.
/// Odometry is updated event-driven on every /wheel_velocity message so the
/// integration step matches the feedback rate of the hardware.
class SuspensionDriverNode : public rclcpp::Node {
public:
  /// Declares parameters, builds the kinematics solver and wires up all
  /// publishers, subscribers and the control-loop timer.
  explicit SuspensionDriverNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

private:
  // --- Subscription callbacks -------------------------------------------

  /// Stores the latest body twist target and refreshes the watchdog stamp.
  void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg);

  /// Stores drive wheel velocities and triggers an odometry update.
  void wheel_velocity_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg);

  /// Stores drive wheel positions (kept for diagnostics/extensions).
  void wheel_position_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg);

  /// Stores the measured steering angles.
  void steering_position_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg);

  // --- Periodic work ------------------------------------------------------

  /// Control loop: watchdog check, inverse kinematics, command publishing.
  void control_loop();

  /// Forward kinematics + pose integration + /wheel_odom (and TF) publishing.
  void update_odometry(const rclcpp::Time &stamp);

  // --- Helpers -------------------------------------------------------------

  /// Validates that a feedback array carries exactly kNumWheels values.
  /// Logs (throttled) and returns std::nullopt on malformed input.
  std::optional<std::array<double, kNumWheels>>
  to_wheel_array(const std_msgs::msg::Float64MultiArray &msg, const char *topic_name);

  /// Publishes zero drive velocities while holding current steering angles.
  void publish_stop_command();

  // --- Kinematics ----------------------------------------------------------

  std::unique_ptr<SwerveKinematics> kinematics_;

  // --- Parameters ----------------------------------------------------------

  double max_drive_velocity_{ 0.0 }; ///< Per-wheel speed limit [rad/s], 0 disables.
  double cmd_vel_timeout_s_{ 0.5 };  ///< Watchdog timeout [s].
  bool publish_tf_{ true };          ///< Whether to broadcast odom -> base TF.
  std::string odom_frame_id_;
  std::string base_frame_id_;

  // --- Shared state (guarded by mutex_) -------------------------------------

  std::mutex mutex_;
  BodyVelocity target_twist_;                            ///< Latest /cmd_vel.
  std::optional<rclcpp::Time> last_cmd_time_;            ///< Watchdog stamp.
  WheelStates wheel_states_;                             ///< Latest feedback.
  std::array<double, kNumWheels> wheel_positions_rad_{}; ///< Latest /wheel_pos.
  bool steering_feedback_received_{ false };
  Pose2D odom_pose_;                           ///< Integrated pose.
  BodyVelocity odom_twist_;                    ///< Latest FK estimate.
  std::optional<rclcpp::Time> last_odom_time_; ///< Last integration stamp.

  // --- ROS interfaces --------------------------------------------------------

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_velocity_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_position_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr steering_position_sub_;

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr drive_cmd_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr steering_cmd_pub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  rclcpp::TimerBase::SharedPtr control_timer_;
};

} // namespace suspension_driver

#endif // SUSPENSION_DRIVER__SUSPENSION_DRIVER_NODE_HPP_
