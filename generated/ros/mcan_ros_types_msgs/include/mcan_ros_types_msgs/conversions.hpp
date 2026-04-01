// Auto-generated CAN frame encoder/decoder for ROS messages
//
// DO NOT EDIT MANUALLY
//

#pragma once

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/u_int8.hpp>
#include <std_msgs/msg/u_int16.hpp>
#include <std_msgs/msg/u_int32.hpp>
#include <std_msgs/msg/u_int64.hpp>
#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/int16.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/int64.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/float64.hpp>
#include <std_msgs/msg/bool.hpp>

// ROS message includes
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <sensor_msgs/msg/imu.hpp>

// C++ type includes
#include <mcan_ros_types_msgs/mcan_ros_types_types.hpp>

namespace mcan_ros_types_msgs {

/**
 * Message conversion utilities for CAN frames and ROS messages.
 * All methods are static inline for use as template parameters.
 *
 * Four conversion types:
 * 1. mcan_encode_val_to_ros() - Primitive values to std_msgs wrappers
 * 2. mcan_decode_type_to_ros() - C++ custom types to ROS type messages
 * 3. mcan_decode_message_to_ros() - Module messages to ROS messages (with header)
 * 4. mcan_encode_ros_to_message() - ROS messages to module messages
 */
struct Conversions {

  // ========== Primitive Value to ROS Wrapper ==========

  static inline std_msgs::msg::UInt8 mcan_encode_val_to_ros(const std::uint8_t &val) {
    std_msgs::msg::UInt8 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::UInt16 mcan_encode_val_to_ros(const std::uint16_t &val) {
    std_msgs::msg::UInt16 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::UInt32 mcan_encode_val_to_ros(const std::uint32_t &val) {
    std_msgs::msg::UInt32 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::UInt64 mcan_encode_val_to_ros(const std::uint64_t &val) {
    std_msgs::msg::UInt64 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Int8 mcan_encode_val_to_ros(const std::int8_t &val) {
    std_msgs::msg::Int8 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Int16 mcan_encode_val_to_ros(const std::int16_t &val) {
    std_msgs::msg::Int16 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Int32 mcan_encode_val_to_ros(const std::int32_t &val) {
    std_msgs::msg::Int32 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Int64 mcan_encode_val_to_ros(const std::int64_t &val) {
    std_msgs::msg::Int64 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Float32 mcan_encode_val_to_ros(const float &val) {
    std_msgs::msg::Float32 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Float64 mcan_encode_val_to_ros(const double &val) {
    std_msgs::msg::Float64 msg;
    msg.data = val;
    return msg;
  }

  static inline std_msgs::msg::Bool mcan_encode_val_to_ros(const bool &val) {
    std_msgs::msg::Bool msg;
    msg.data = val;
    return msg;
  }


  template <float Scale> static inline double mcan_decode_type_to_ros(const FloatInt16_t<Scale> &val) {
    return (double)val;
  }

  static inline double mcan_encode_ros_to_type(const double &val) {
    return (double)val;
  }

  // ========== Custom Type Conversions ==========

  /**
   * Convert C++ custom type to ROS type message
   * @param val C++ type instance
   * @return ROS type message
   * Direct field mapping to geometry_msgs/Quaternion
   */
  static inline geometry_msgs::msg::Quaternion
  mcan_decode_type_to_ros(const mcan::mcan_ros_types::GeometryMsgsQuaternion_t &val) {
    geometry_msgs::msg::Quaternion msg;
    msg.x = mcan_decode_type_to_ros(val.x);
    msg.y = mcan_decode_type_to_ros(val.y);
    msg.z = mcan_decode_type_to_ros(val.z);
    msg.w = mcan_decode_type_to_ros(val.w);
    return msg;
  }

  /**
   * Convert ROS type message to C++ custom type
   * @param msg ROS type message
   * @return C++ type instance
   * Direct field mapping from geometry_msgs/Quaternion
   */
  static inline mcan::mcan_ros_types::GeometryMsgsQuaternion_t
  mcan_encode_ros_to_type(const geometry_msgs::msg::Quaternion &msg) {
    mcan::mcan_ros_types::GeometryMsgsQuaternion_t val;
    val.x = mcan_encode_ros_to_type(msg.x);
    val.y = mcan_encode_ros_to_type(msg.y);
    val.z = mcan_encode_ros_to_type(msg.z);
    val.w = mcan_encode_ros_to_type(msg.w);
    return val;
  }

  /**
   * Convert C++ custom type to ROS type message
   * @param val C++ type instance
   * @return ROS type message
   * Direct field mapping to geometry_msgs/Vector3
   */
  static inline geometry_msgs::msg::Vector3
  mcan_decode_type_to_ros(const mcan::mcan_ros_types::GeometryMsgsVector3_t &val) {
    geometry_msgs::msg::Vector3 msg;
    msg.x = mcan_decode_type_to_ros(val.x);
    msg.y = mcan_decode_type_to_ros(val.y);
    msg.z = mcan_decode_type_to_ros(val.z);
    return msg;
  }

  /**
   * Convert ROS type message to C++ custom type
   * @param msg ROS type message
   * @return C++ type instance
   * Direct field mapping from geometry_msgs/Vector3
   */
  static inline mcan::mcan_ros_types::GeometryMsgsVector3_t
  mcan_encode_ros_to_type(const geometry_msgs::msg::Vector3 &msg) {
    mcan::mcan_ros_types::GeometryMsgsVector3_t val;
    val.x = mcan_encode_ros_to_type(msg.x);
    val.y = mcan_encode_ros_to_type(msg.y);
    val.z = mcan_encode_ros_to_type(msg.z);
    return val;
  }

  /**
   * Convert C++ custom type to ROS type message
   * @param val C++ type instance
   * @return ROS type message
   * Direct field mapping to sensor_msgs/Imu
   */
  static inline sensor_msgs::msg::Imu mcan_decode_type_to_ros(const mcan::mcan_ros_types::SensorMsgsImu_t &val) {
    sensor_msgs::msg::Imu msg;
    msg.orientation      = mcan_ros_types_msgs::Conversions::mcan_decode_type_to_ros(val.orientation);
    msg.angular_velocity = mcan_ros_types_msgs::Conversions::mcan_decode_type_to_ros(val.angular_velocity);
    msg.linear_acceleration = mcan_ros_types_msgs::Conversions::mcan_decode_type_to_ros(val.linear_acceleration);
    return msg;
  }

  /**
   * Convert ROS type message to C++ custom type
   * @param msg ROS type message
   * @return C++ type instance
   * Direct field mapping from sensor_msgs/Imu
   */
  static inline mcan::mcan_ros_types::SensorMsgsImu_t mcan_encode_ros_to_type(const sensor_msgs::msg::Imu &msg) {
    mcan::mcan_ros_types::SensorMsgsImu_t val;
    val.orientation      = mcan_ros_types_msgs::Conversions::mcan_encode_ros_to_type(msg.orientation);
    val.angular_velocity = mcan_ros_types_msgs::Conversions::mcan_encode_ros_to_type(msg.angular_velocity);
    val.linear_acceleration = mcan_ros_types_msgs::Conversions::mcan_encode_ros_to_type(msg.linear_acceleration);
    return val;
  }

  // ========== Module Message Conversions ==========

}; // struct Conversions

} // namespace mcan_ros_types_msgs
