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

BodyVelocity SwerveKinematics::forward(const WheelStates &wheels) const {
  // Each wheel contributes two linear equations in the unknowns
  // q = [vx, vy, omega]^T:
  //
  //   v_i * cos(phi_i) = vx - omega * y_i
  //   v_i * sin(phi_i) = vy + omega * x_i
  //
  // with v_i the measured ground speed and phi_i the measured steering
  // angle. Eight equations and three unknowns are solved via the normal
  // equations (A^T A) q = A^T b of the least-squares problem, which
  // averages out encoder noise and wheel slip. For the standard symmetric
  // wheel layout A^T A is well conditioned and the closed-form 3x3 solve
  // below is exact and allocation-free.

  double sum_x  = 0.0; // sum of x_i
  double sum_y  = 0.0; // sum of y_i
  double sum_r2 = 0.0; // sum of (x_i^2 + y_i^2)
  double b0     = 0.0; // sum of v_xi
  double b1     = 0.0; // sum of v_yi
  double b2     = 0.0; // sum of (x_i * v_yi - y_i * v_xi)

  for(std::size_t i = 0; i < kNumWheels; ++i) {
    const double x    = wheel_positions_[i][0];
    const double y    = wheel_positions_[i][1];
    const double v    = wheels.drive_velocities[i] * wheel_radius_;
    const double v_xi = v * std::cos(wheels.steering_angles[i]);
    const double v_yi = v * std::sin(wheels.steering_angles[i]);

    sum_x += x;
    sum_y += y;
    sum_r2 += x * x + y * y;
    b0 += v_xi;
    b1 += v_yi;
    b2 += x * v_yi - y * v_xi;
  }

  // Normal-equation matrix M = A^T A (symmetric):
  //   [ N     0    -sum_y ]
  //   [ 0     N     sum_x ]
  //   [-sum_y sum_x sum_r2]
  const double n = static_cast<double>(kNumWheels);

  // Closed-form solve via Cramer's rule on the 3x3 symmetric system.
  const double det = n * (n * sum_r2 - sum_x * sum_x - sum_y * sum_y);
  if(std::abs(det) < 1e-12) {
    // Degenerate geometry (all wheels at the origin); cannot happen with a
    // validated RobotGeometry but guard the division regardless.
    return BodyVelocity{};
  }

  // Solve q = M^{-1} (A^T b) through the (symmetric) adjugate of
  // M = [[n, 0, -sy], [0, n, sx], [-sy, sx, srr]]:
  const double sx  = sum_x;
  const double sy  = sum_y;
  const double srr = sum_r2;

  const double c00 = n * srr - sx * sx;
  const double c01 = -sx * sy;
  const double c02 = n * sy;
  const double c11 = n * srr - sy * sy;
  const double c12 = -n * sx;
  const double c22 = n * n;
  BodyVelocity body;
  body.vx    = (c00 * b0 + c01 * b1 + c02 * b2) / det;
  body.vy    = (c01 * b0 + c11 * b1 + c12 * b2) / det;
  body.omega = (c02 * b0 + c12 * b1 + c22 * b2) / det;
  return body;
}

void SwerveKinematics::integrate(Pose2D &pose, const BodyVelocity &body, double dt) {
  if(dt <= 0.0) {
    return;
  }

  const double yaw_next = pose.yaw + body.omega * dt;

  if(std::abs(body.omega) < kOmegaEpsilon) {
    // Straight-line motion: rotate the body twist into the odom frame and
    // integrate linearly.
    const double cos_yaw = std::cos(pose.yaw);
    const double sin_yaw = std::sin(pose.yaw);
    pose.x += (body.vx * cos_yaw - body.vy * sin_yaw) * dt;
    pose.y += (body.vx * sin_yaw + body.vy * cos_yaw) * dt;
  } else {
    // Exact integration of a constant twist: the base travels along a
    // circular arc. This is more accurate than Euler/Runge-Kutta at the
    // typical feedback rates of a mobile base.
    const double dsin = std::sin(yaw_next) - std::sin(pose.yaw);
    const double dcos = std::cos(yaw_next) - std::cos(pose.yaw);
    pose.x += (body.vx * dsin + body.vy * dcos) / body.omega;
    pose.y += (body.vx * -dcos + body.vy * dsin) / body.omega;
  }

  pose.yaw = normalize_angle(yaw_next);
}

} // namespace suspension_driver
