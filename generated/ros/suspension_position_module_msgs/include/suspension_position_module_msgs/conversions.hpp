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
#include <suspension_position_module_msgs/msg/discover_devices.hpp>
#include <suspension_position_module_msgs/msg/set_device_node_id.hpp>
#include <suspension_position_module_msgs/msg/enter_configuration_mode.hpp>
#include <suspension_position_module_msgs/msg/get_hardware_type.hpp>
#include <suspension_position_module_msgs/msg/ping_module.hpp>
#include <suspension_position_module_msgs/msg/flash_indicator_led.hpp>
#include <suspension_position_module_msgs/msg/set_encoder_offset_left.hpp>
#include <suspension_position_module_msgs/msg/set_encoder_offset_right.hpp>
#include <suspension_position_module_msgs/msg/current_position_left.hpp>
#include <suspension_position_module_msgs/msg/current_position_right.hpp>

// C++ type includes
#include <suspension_position_module_msgs/suspension_position_module_types.hpp>
#include <suspension_position_module_msgs/suspension_position_module.hpp>

// Dependency conversions
#include <mcan_basic_module_msgs/conversions.hpp>
#include <mcan_ros_types_msgs/conversions.hpp>

namespace suspension_position_module_msgs {

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

  // ========== Module Message Conversions ==========

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::DiscoverDevices
  mcan_decode_message_to_ros(const mcan::suspension_position_module::configs::DiscoverDevices &msg_struct) {
    msg::DiscoverDevices msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::configs::DiscoverDevices
  mcan_encode_ros_to_message(const msg::DiscoverDevices &msg) {
    mcan::suspension_position_module::configs::DiscoverDevices msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::SetDeviceNodeId
  mcan_decode_message_to_ros(const mcan::suspension_position_module::configs::SetDeviceNodeId &msg_struct) {
    msg::SetDeviceNodeId msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::configs::SetDeviceNodeId
  mcan_encode_ros_to_message(const msg::SetDeviceNodeId &msg) {
    mcan::suspension_position_module::configs::SetDeviceNodeId msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::EnterConfigurationMode
  mcan_decode_message_to_ros(const mcan::suspension_position_module::configs::EnterConfigurationMode &msg_struct) {
    msg::EnterConfigurationMode msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::configs::EnterConfigurationMode
  mcan_encode_ros_to_message(const msg::EnterConfigurationMode &msg) {
    mcan::suspension_position_module::configs::EnterConfigurationMode msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::GetHardwareType
  mcan_decode_message_to_ros(const mcan::suspension_position_module::configs::GetHardwareType &msg_struct) {
    msg::GetHardwareType msg;
    msg.data = mcan_basic_module_msgs::Conversions::mcan_decode_type_to_ros(msg_struct.value);
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::configs::GetHardwareType
  mcan_encode_ros_to_message(const msg::GetHardwareType &msg) {
    mcan::suspension_position_module::configs::GetHardwareType msg_struct;
    msg_struct.value = mcan_basic_module_msgs::Conversions::mcan_encode_ros_to_type(msg.data);
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::PingModule
  mcan_decode_message_to_ros(const mcan::suspension_position_module::configs::PingModule &msg_struct) {
    msg::PingModule msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::configs::PingModule
  mcan_encode_ros_to_message(const msg::PingModule &msg) {
    mcan::suspension_position_module::configs::PingModule msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::FlashIndicatorLed
  mcan_decode_message_to_ros(const mcan::suspension_position_module::configs::FlashIndicatorLed &msg_struct) {
    msg::FlashIndicatorLed msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::configs::FlashIndicatorLed
  mcan_encode_ros_to_message(const msg::FlashIndicatorLed &msg) {
    mcan::suspension_position_module::configs::FlashIndicatorLed msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::SetEncoderOffsetLeft
  mcan_decode_message_to_ros(const mcan::suspension_position_module::commands::SetEncoderOffsetLeft &msg_struct) {
    msg::SetEncoderOffsetLeft msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::commands::SetEncoderOffsetLeft
  mcan_encode_ros_to_message(const msg::SetEncoderOffsetLeft &msg) {
    mcan::suspension_position_module::commands::SetEncoderOffsetLeft msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::SetEncoderOffsetRight
  mcan_decode_message_to_ros(const mcan::suspension_position_module::commands::SetEncoderOffsetRight &msg_struct) {
    msg::SetEncoderOffsetRight msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::commands::SetEncoderOffsetRight
  mcan_encode_ros_to_message(const msg::SetEncoderOffsetRight &msg) {
    mcan::suspension_position_module::commands::SetEncoderOffsetRight msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::CurrentPositionLeft
  mcan_decode_message_to_ros(const mcan::suspension_position_module::states::CurrentPositionLeft &msg_struct) {
    msg::CurrentPositionLeft msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::states::CurrentPositionLeft
  mcan_encode_ros_to_message(const msg::CurrentPositionLeft &msg) {
    mcan::suspension_position_module::states::CurrentPositionLeft msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

  /**
   * Convert C++ module message struct to ROS message
   * @param msg_struct C++ module message struct (from main header)
   * @return ROS message with data (header not initialized)
   */
  static inline msg::CurrentPositionRight
  mcan_decode_message_to_ros(const mcan::suspension_position_module::states::CurrentPositionRight &msg_struct) {
    msg::CurrentPositionRight msg;
    msg.data = msg_struct.value;
    return msg;
  }

  /**
   * Convert ROS message to C++ module message struct (extracts data, ignores header)
   * @param msg ROS message instance
   * @return C++ module message struct (from main header)
   */
  static inline mcan::suspension_position_module::states::CurrentPositionRight
  mcan_encode_ros_to_message(const msg::CurrentPositionRight &msg) {
    mcan::suspension_position_module::states::CurrentPositionRight msg_struct;
    msg_struct.value = msg.data;
    return msg_struct;
  }

}; // struct Conversions

} // namespace suspension_position_module_msgs
