// Auto-generated types for mcan_ros_types_msgs ROS package
//
// DO NOT EDIT MANUALLY
//

#pragma once

#include <cstdint>
#include <string>
#include <cstddef>
#include "mc_plugin_base/mc_firmware/super_types.hpp"

namespace mcan {

namespace mcan_ros_types {

// Forward declarations
struct GeometryMsgsQuaternion_t;
struct GeometryMsgsVector3_t;
struct SensorMsgsImu_t;

// Type definitions

/**
 * @brief Quaternion
 * Representation of orientation in 3D space
 */
struct GeometryMsgsQuaternion_t {

  /**
   * @brief X component of the quaternion
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 0.0
   */
  FloatInt16_t<10000.0f> x = 0.0;

  /**
   * @brief Y component of the quaternion
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 0.0
   */
  FloatInt16_t<10000.0f> y = 0.0;

  /**
   * @brief Z component of the quaternion
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 0.0
   */
  FloatInt16_t<10000.0f> z = 0.0;

  /**
   * @brief W component of the quaternion
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 1.0
   */
  FloatInt16_t<10000.0f> w = 1.0;
};

/**
 * @brief Vector3
 * Representation of a vector in 3D space
 */
struct GeometryMsgsVector3_t {

  /**
   * @brief X component of the vector
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 0.0
   */
  FloatInt16_t<10000.0f> x = 0.0;

  /**
   * @brief Y component of the vector
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 0.0
   */
  FloatInt16_t<10000.0f> y = 0.0;

  /**
   * @brief Z component of the vector
   * @type FloatInt16_t<10000.0f>
   * @units unitless
   * @default 0.0
   */
  FloatInt16_t<10000.0f> z = 0.0;
};

/**
 * @brief IMU Data
 * Inertial Measurement Unit Data
 */
struct SensorMsgsImu_t {

  /**
   * @brief Orientation as a quaternion (x, y, z, w)
   * @type GeometryMsgsQuaternion_t
   * @units unitless
   * @default {0,0,0,1}
   */
  GeometryMsgsQuaternion_t orientation = { 0, 0, 0, 1 };

  /**
   * @brief Angular velocity (x, y, z)
   * @type GeometryMsgsVector3_t
   * @units rad/s
   * @default {0,0,0}
   */
  GeometryMsgsVector3_t angular_velocity = { 0, 0, 0 };

  /**
   * @brief Linear acceleration (x, y, z)
   * @type GeometryMsgsVector3_t
   * @units m/s^2
   * @default {0,0,0}
   */
  GeometryMsgsVector3_t linear_acceleration = { 0, 0, 0 };
};

} // namespace mcan_ros_types

} // namespace mcan