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

#include <cmath>
#include <numbers>

#include <gtest/gtest.h>

#include "suspension_driver/swerve_kinematics.hpp"

namespace {

using suspension_driver::BodyVelocity;
using suspension_driver::kNumWheels;
using suspension_driver::Pose2D;
using suspension_driver::RobotGeometry;
using suspension_driver::SwerveKinematics;
using suspension_driver::WheelStates;

constexpr double kTol = 1e-9;

RobotGeometry test_geometry() {
  RobotGeometry geometry;
  geometry.wheel_radius = 0.1;
  geometry.wheel_base   = 0.6;
  geometry.track_width  = 0.4;
  return geometry;
}

TEST(SwerveKinematics, RejectsInvalidGeometry) {
  RobotGeometry geometry = test_geometry();
  geometry.wheel_radius  = 0.0;
  EXPECT_THROW(SwerveKinematics{ geometry }, std::invalid_argument);
}

TEST(SwerveKinematics, NormalizeAngle) {
  EXPECT_NEAR(SwerveKinematics::normalize_angle(0.0), 0.0, kTol);
  EXPECT_NEAR(SwerveKinematics::normalize_angle(3.0 * std::numbers::pi), std::numbers::pi, kTol);
  EXPECT_NEAR(SwerveKinematics::normalize_angle(-0.5 * std::numbers::pi), -0.5 * std::numbers::pi, kTol);
  EXPECT_NEAR(SwerveKinematics::normalize_angle(2.0 * std::numbers::pi + 0.1), 0.1, kTol);
}

TEST(SwerveKinematics, InversePureForward) {
  const SwerveKinematics kinematics{ test_geometry() };
  const auto commands = kinematics.inverse(BodyVelocity{ 1.0, 0.0, 0.0 }, {});

  for(std::size_t i = 0; i < kNumWheels; ++i) {
    EXPECT_NEAR(commands.steering_angles[i], 0.0, kTol);
    EXPECT_NEAR(commands.drive_velocities[i], 1.0 / 0.1, kTol);
  }
}

TEST(SwerveKinematics, InversePureRotationIsTangential) {
  const SwerveKinematics kinematics{ test_geometry() };
  const double omega  = 1.0;
  const auto commands = kinematics.inverse(BodyVelocity{ 0.0, 0.0, omega }, {});

  const double radius = std::hypot(0.3, 0.2); // Distance module <-> base center.
  for(std::size_t i = 0; i < kNumWheels; ++i) {
    EXPECT_NEAR(std::abs(commands.drive_velocities[i]), omega * radius / 0.1, 1e-6);
  }
  // Front-left module: contact velocity (-omega*y, omega*x) = (-0.2, 0.3),
  // i.e. a raw heading of atan2(0.3, -0.2) ~ 124 deg. That is more than
  // 90 deg away from the current angle (0), so the optimization steers to
  // the supplementary angle and reverses the drive direction instead.
  EXPECT_NEAR(commands.steering_angles[0],
              SwerveKinematics::normalize_angle(std::atan2(0.3, -0.2) - std::numbers::pi), kTol);
  EXPECT_LT(commands.drive_velocities[0], 0.0);
}

TEST(SwerveKinematics, InverseReversesInsteadOfLargeSteer) {
  const SwerveKinematics kinematics{ test_geometry() };
  // Driving backwards while modules point forwards: expect the optimization
  // to keep the steering at 0 and spin the wheels backwards.
  const auto commands = kinematics.inverse(BodyVelocity{ -1.0, 0.0, 0.0 }, {});

  for(std::size_t i = 0; i < kNumWheels; ++i) {
    EXPECT_NEAR(commands.steering_angles[i], 0.0, kTol);
    EXPECT_NEAR(commands.drive_velocities[i], -1.0 / 0.1, kTol);
  }
}

TEST(SwerveKinematics, InverseZeroTwistHoldsSteering) {
  const SwerveKinematics kinematics{ test_geometry() };
  const std::array<double, kNumWheels> held{ 0.3, -0.4, 1.2, -2.0 };
  const auto commands = kinematics.inverse(BodyVelocity{}, held);

  for(std::size_t i = 0; i < kNumWheels; ++i) {
    EXPECT_NEAR(commands.steering_angles[i], held[i], kTol);
    EXPECT_NEAR(commands.drive_velocities[i], 0.0, kTol);
  }
}

TEST(SwerveKinematics, ForwardInverseRoundTrip) {
  const SwerveKinematics kinematics{ test_geometry() };
  const BodyVelocity twist{ 0.7, -0.25, 0.9 };

  // Seed current steering with the exact IK solution so the optimization
  // does not flip any module; FK must then recover the same twist.
  auto seed           = kinematics.inverse(twist, {});
  const auto commands = kinematics.inverse(twist, seed.steering_angles);

  WheelStates states;
  states.drive_velocities = commands.drive_velocities;
  states.steering_angles  = commands.steering_angles;

  const BodyVelocity estimate = kinematics.forward(states);
  EXPECT_NEAR(estimate.vx, twist.vx, 1e-6);
  EXPECT_NEAR(estimate.vy, twist.vy, 1e-6);
  EXPECT_NEAR(estimate.omega, twist.omega, 1e-6);
}

TEST(SwerveKinematics, IntegrateStraightLine) {
  Pose2D pose;
  pose.yaw = 0.5 * std::numbers::pi; // Facing +y.
  SwerveKinematics::integrate(pose, BodyVelocity{ 1.0, 0.0, 0.0 }, 2.0);

  EXPECT_NEAR(pose.x, 0.0, 1e-9);
  EXPECT_NEAR(pose.y, 2.0, 1e-9);
  EXPECT_NEAR(pose.yaw, 0.5 * std::numbers::pi, kTol);
}

TEST(SwerveKinematics, IntegrateQuarterArc) {
  // Constant twist (v, omega) drives a circle of radius v/omega. A quarter
  // turn from the origin facing +x must end at (R, R) facing +y.
  Pose2D pose;
  const double v     = 1.0;
  const double omega = 0.5;
  const double dt    = (0.5 * std::numbers::pi) / omega;
  SwerveKinematics::integrate(pose, BodyVelocity{ v, 0.0, omega }, dt);

  const double radius = v / omega;
  EXPECT_NEAR(pose.x, radius, 1e-9);
  EXPECT_NEAR(pose.y, radius, 1e-9);
  EXPECT_NEAR(pose.yaw, 0.5 * std::numbers::pi, kTol);
}

TEST(SwerveKinematics, IntegrateIgnoresNonPositiveDt) {
  Pose2D pose;
  pose.x = 1.0;
  SwerveKinematics::integrate(pose, BodyVelocity{ 1.0, 1.0, 1.0 }, -0.1);
  EXPECT_NEAR(pose.x, 1.0, kTol);
  EXPECT_NEAR(pose.y, 0.0, kTol);
}

} // namespace
