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
#include <cmath>
#include <functional>

#include "geometry_msgs/msg/transform_stamped.hpp"

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

  // --- Publishers ------------------------------------------------------------
  drive_cmd_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>("/drive_velocity_cmd", rclcpp::QoS(1));
  steering_cmd_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>("/steering_position_cmd", rclcpp::QoS(1));
  odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/wheel_odom", rclcpp::QoS(10));
  if(publish_tf_) {
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
  }

  // --- Control loop ------------------------------------------------------------
  const auto control_period = std::chrono::duration<double>(1.0 / std::max(control_rate_hz, 1.0));
  control_timer_ = create_wall_timer(std::chrono::duration_cast<std::chrono::nanoseconds>(control_period),
                                     std::bind(&SuspensionDriverNode::control_loop, this));

  RCLCPP_INFO(get_logger(),
              "suspension_driver up: wheel_radius=%.3f m, wheel_base=%.3f m, track_width=%.3f m, "
              "control_rate=%.1f Hz",
              geometry.wheel_radius, geometry.wheel_base, geometry.track_width, control_rate_hz);
}

void SuspensionDriverNode::control_loop() {
  BodyVelocity target;
  std::array<double, kNumWheels> current_steering{};
  bool have_command = false;

  {
    const std::lock_guard<std::mutex> lock(mutex_);
    // Watchdog: only forward commands while /cmd_vel is fresh. A navigation
    // stack crash or Wi-Fi drop must never leave the base driving blind.
    have_command = last_cmd_time_.has_value() && (now() - *last_cmd_time_).seconds() <= cmd_vel_timeout_s_;
    target       = target_twist_;
    current_steering = wheel_states_.steering_angles;
  }

  if(!have_command) {
    publish_stop_command();
    return;
  }

  WheelCommands commands = kinematics_->inverse(target, current_steering);

  // Optional per-wheel speed limit. Scale the whole command set uniformly so
  // the base follows the commanded direction, just slower (never distorts
  // the intended motion the way per-wheel clamping would).
  if(max_drive_velocity_ > 0.0) {
    double max_requested = 0.0;
    for(const double v : commands.drive_velocities) {
      max_requested = std::max(max_requested, std::abs(v));
    }
    if(max_requested > max_drive_velocity_) {
      const double scale = max_drive_velocity_ / max_requested;
      for(double &v : commands.drive_velocities) {
        v *= scale;
      }
    }
  }

  std_msgs::msg::Float64MultiArray drive_msg;
  drive_msg.data.assign(commands.drive_velocities.begin(), commands.drive_velocities.end());
  drive_cmd_pub_->publish(drive_msg);

  std_msgs::msg::Float64MultiArray steering_msg;
  steering_msg.data.assign(commands.steering_angles.begin(), commands.steering_angles.end());
  steering_cmd_pub_->publish(steering_msg);
}

void SuspensionDriverNode::publish_stop_command() {
  std::array<double, kNumWheels> current_steering{};
  {
    const std::lock_guard<std::mutex> lock(mutex_);
    current_steering = wheel_states_.steering_angles;
  }

  // Zero drive velocity, hold current steering: the safest reachable state
  // for a swerve base (no wheel fighting, no unexpected module rotation).
  std_msgs::msg::Float64MultiArray drive_msg;
  drive_msg.data.assign(kNumWheels, 0.0);
  drive_cmd_pub_->publish(drive_msg);

  std_msgs::msg::Float64MultiArray steering_msg;
  steering_msg.data.assign(current_steering.begin(), current_steering.end());
  steering_cmd_pub_->publish(steering_msg);
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
  {
    const std::lock_guard<std::mutex> lock(mutex_);
    wheel_states_.drive_velocities = *values;
  }
  // Drive velocity is the fastest-changing measurement, so each new sample
  // drives one odometry integration step (dt = time between samples).
  update_odometry(now());
}

void SuspensionDriverNode::update_odometry(const rclcpp::Time &stamp) {
  Pose2D pose;
  BodyVelocity twist;

  {
    const std::lock_guard<std::mutex> lock(mutex_);

    // Without steering feedback the wheel headings are unknown and forward
    // kinematics would integrate garbage; wait for the first sample.
    if(!steering_feedback_received_) {
      return;
    }

    twist       = kinematics_->forward(wheel_states_);
    odom_twist_ = twist;

    if(last_odom_time_.has_value()) {
      const double dt = (stamp - *last_odom_time_).seconds();
      // Reject non-monotonic stamps and gaps (e.g. feedback dropouts or a
      // sim-time reset); resume integrating from the next sample.
      if(dt > 0.0 && dt < 1.0) {
        SwerveKinematics::integrate(odom_pose_, twist, dt);
      }
    }
    last_odom_time_ = stamp;
    pose            = odom_pose_;
  }

  nav_msgs::msg::Odometry odom;
  odom.header.stamp    = stamp;
  odom.header.frame_id = odom_frame_id_;
  odom.child_frame_id  = base_frame_id_;

  odom.pose.pose.position.x    = pose.x;
  odom.pose.pose.position.y    = pose.y;
  odom.pose.pose.orientation.z = std::sin(0.5 * pose.yaw);
  odom.pose.pose.orientation.w = std::cos(0.5 * pose.yaw);

  odom.twist.twist.linear.x  = twist.vx;
  odom.twist.twist.linear.y  = twist.vy;
  odom.twist.twist.angular.z = twist.omega;

  // Conservative fixed covariance diagonal for a wheel-odometry source; a
  // downstream EKF (e.g. robot_localization) should treat this as one input.
  constexpr double kPoseVariance  = 1e-3;
  constexpr double kTwistVariance = 1e-3;
  for(const std::size_t i : { 0, 7, 14, 21, 28, 35 }) {
    odom.pose.covariance[i]  = kPoseVariance;
    odom.twist.covariance[i] = kTwistVariance;
  }

  odom_pub_->publish(odom);

  if(tf_broadcaster_) {
    geometry_msgs::msg::TransformStamped tf;
    tf.header.stamp            = stamp;
    tf.header.frame_id         = odom_frame_id_;
    tf.child_frame_id          = base_frame_id_;
    tf.transform.translation.x = pose.x;
    tf.transform.translation.y = pose.y;
    tf.transform.rotation.z    = std::sin(0.5 * pose.yaw);
    tf.transform.rotation.w    = std::cos(0.5 * pose.yaw);
    tf_broadcaster_->sendTransform(tf);
  }
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
