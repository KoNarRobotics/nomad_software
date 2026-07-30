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

#include "suspension_driver/swerve_kinematics.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace suspension_driver {

namespace {
/// Velocities below this magnitude [m/s] are treated as zero so steering
/// angles are held instead of being derived from numerical noise.
constexpr double kVelocityEpsilon = 1e-6;

/// Yaw rates below this magnitude [rad/s] use the straight-line pose
/// integration branch to avoid dividing by a near-zero omega.
constexpr double kOmegaEpsilon = 1e-9;
} // namespace

SwerveKinematics::SwerveKinematics(const RobotGeometry &geometry) : wheel_radius_(geometry.wheel_radius) {
  if(geometry.wheel_radius <= 0.0 || geometry.wheel_base <= 0.0 || geometry.track_width <= 0.0) {
    throw std::invalid_argument("SwerveKinematics: all geometry dimensions must be > 0");
  }

  const double half_base  = 0.5 * geometry.wheel_base;
  const double half_track = 0.5 * geometry.track_width;

  // Body frame: x forward, y left (REP-103). Order: FL, FR, RL, RR.
  wheel_positions_[kFrontLeft]  = { half_base, half_track };
  wheel_positions_[kFrontRight] = { half_base, -half_track };
  wheel_positions_[kRearLeft]   = { -half_base, half_track };
  wheel_positions_[kRearRight]  = { -half_base, -half_track };
}

double SwerveKinematics::normalize_angle(double angle) {
  // Wrap into (-pi, pi] using remainder(), which returns a value in
  // [-pi, pi]; map the single -pi edge case onto +pi.
  const double wrapped = std::remainder(angle, 2.0 * std::numbers::pi);
  return (wrapped <= -std::numbers::pi) ? std::numbers::pi : wrapped;
}

WheelCommands SwerveKinematics::inverse(const BodyVelocity &body,
                                        const std::array<double, kNumWheels> &current_steering_angles) const {
  WheelCommands commands;

  for(std::size_t i = 0; i < kNumWheels; ++i) {
    const double x = wheel_positions_[i][0];
    const double y = wheel_positions_[i][1];

    // Rigid body velocity of the wheel contact point:
    // v_i = v_base + omega x r_i  (planar cross product).
    const double vx_i = body.vx - body.omega * y;
    const double vy_i = body.vy + body.omega * x;

    const double speed = std::hypot(vx_i, vy_i);

    if(speed < kVelocityEpsilon) {
      // No meaningful motion for this module: stop the drive and hold the
      // current steering angle so the module does not snap to zero.
      commands.drive_velocities[i] = 0.0;
      commands.steering_angles[i]  = current_steering_angles[i];
      continue;
    }

    double steering_angle = std::atan2(vy_i, vx_i);
    double wheel_velocity = speed / wheel_radius_;

    // Swerve optimization: never steer more than 90 deg. If the target
    // heading is in the opposite half-plane, steer to the supplementary
    // angle and reverse the drive direction instead.
    const double error = normalize_angle(steering_angle - current_steering_angles[i]);
    if(std::abs(error) > 0.5 * std::numbers::pi) {
      steering_angle = normalize_angle(steering_angle + std::numbers::pi);
      wheel_velocity = -wheel_velocity;
    }

    commands.drive_velocities[i] = wheel_velocity;
    commands.steering_angles[i]  = steering_angle;
  }

  return commands;
}

} // namespace suspension_driver
