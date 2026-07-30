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

#ifndef SUSPENSION_DRIVER__SWERVE_KINEMATICS_HPP_
#define SUSPENSION_DRIVER__SWERVE_KINEMATICS_HPP_

#include <array>
#include <cstddef>

/// Pure (ROS-free) kinematics for a 4-wheel independent-drive,
/// independent-steering (swerve) mobile robot.
///
/// Conventions (REP-103):
///   * Body frame: x forward, y left, z up. Yaw is CCW-positive.
///   * Wheel order everywhere: FL, FR, RL, RR.
///   * Steering angle 0 points the wheel along +x; positive is CCW.
///   * Drive velocities are wheel angular velocities [rad/s];
///     positive spins the wheel so the contact point moves along the
///     wheel heading.
///
/// Reference: https://control.ros.org/humble/doc/ros2_controllers/doc/mobile_robot_kinematics.html
namespace suspension_driver {

/// Number of wheel modules on the robot.
inline constexpr std::size_t kNumWheels = 4;

/// Index of each wheel module inside every std::array used by this library.
enum WheelIndex : std::size_t {
  kFrontLeft  = 0,
  kFrontRight = 1,
  kRearLeft   = 2,
  kRearRight  = 3,
};

/// Planar body-frame velocity (twist) of the robot base.
struct BodyVelocity {
  double vx{ 0.0 };    ///< Longitudinal velocity [m/s].
  double vy{ 0.0 };    ///< Lateral velocity [m/s].
  double omega{ 0.0 }; ///< Yaw rate [rad/s].
};

/// Planar pose of the robot base in the odometry frame.
struct Pose2D {
  double x{ 0.0 };   ///< Position x [m].
  double y{ 0.0 };   ///< Position y [m].
  double yaw{ 0.0 }; ///< Heading [rad], normalized to (-pi, pi].
};

/// Per-wheel command produced by inverse kinematics.
struct WheelCommands {
  std::array<double, kNumWheels> drive_velocities{}; ///< Wheel angular velocities [rad/s].
  std::array<double, kNumWheels> steering_angles{};  ///< Steering positions [rad].
};

/// Per-wheel measured state consumed by forward kinematics.
struct WheelStates {
  std::array<double, kNumWheels> drive_velocities{}; ///< Wheel angular velocities [rad/s].
  std::array<double, kNumWheels> steering_angles{};  ///< Steering positions [rad].
};

/// Geometric description of the drive base.
struct RobotGeometry {
  double wheel_radius{ 0.1 }; ///< Drive wheel radius [m].
  double wheel_base{ 0.5 };   ///< Longitudinal distance between front and rear axles [m].
  double track_width{ 0.5 };  ///< Lateral distance between left and right wheels [m].
};

/// Stateless swerve-drive kinematics solver.
///
/// The class is immutable after construction which makes it trivially
/// thread-safe; all solver methods are const and re-entrant.
class SwerveKinematics {
public:
  /// @param geometry Physical dimensions of the drive base.
  /// @throws std::invalid_argument if any dimension is not strictly positive.
  explicit SwerveKinematics(const RobotGeometry &geometry);

  /// Inverse kinematics: body twist -> per-wheel drive velocity and steering angle.
  ///
  /// Applies the standard swerve optimization: if the target steering angle is
  /// more than 90 deg away from the current one, the module steers to the
  /// supplementary angle and reverses the drive direction instead. When the
  /// commanded twist is (numerically) zero the current steering angles are
  /// held so the modules do not snap back to zero.
  ///
  /// @param body Desired body twist.
  /// @param current_steering_angles Latest measured steering positions [rad].
  /// @return Per-wheel commands.
  WheelCommands inverse(const BodyVelocity &body, const std::array<double, kNumWheels> &current_steering_angles) const;

  /// Forward kinematics: per-wheel measurements -> body twist.
  ///
  /// Solves the overdetermined system (8 equations, 3 unknowns) with a
  /// linear least-squares fit, which averages out sensor noise and wheel
  /// slip across the four modules.
  ///
  /// @param wheels Measured wheel states.
  /// @return Estimated body twist.
  BodyVelocity forward(const WheelStates &wheels) const;

  /// Integrates a body twist over @p dt seconds onto @p pose (odometry).
  ///
  /// Uses exact integration for the constant-twist arc (falls back to a
  /// second-order approximation when |omega| is negligible).
  ///
  /// @param pose Pose to update in place.
  /// @param body Body twist assumed constant over the interval.
  /// @param dt Integration interval [s]; non-positive values are ignored.
  static void integrate(Pose2D &pose, const BodyVelocity &body, double dt);

  /// Normalizes an angle to (-pi, pi].
  static double normalize_angle(double angle);

  /// @return Wheel mounting position (x, y) in the body frame [m].
  const std::array<std::array<double, 2>, kNumWheels> &wheel_positions() const {
    return wheel_positions_;
  }

  /// @return Drive wheel radius [m].
  double wheel_radius() const {
    return wheel_radius_;
  }

private:
  double wheel_radius_;
  /// Wheel mounting points {x, y} in the body frame, ordered FL, FR, RL, RR.
  std::array<std::array<double, 2>, kNumWheels> wheel_positions_;
};

} // namespace suspension_driver

#endif // SUSPENSION_DRIVER__SWERVE_KINEMATICS_HPP_
