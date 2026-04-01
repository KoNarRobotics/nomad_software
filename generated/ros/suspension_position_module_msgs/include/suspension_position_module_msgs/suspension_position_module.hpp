// Auto-generated header for suspension_position_module module
// Do not edit manually

#pragma once

#include <cstdint>
#include <string>
#include <cstddef>
#include <tuple>
#include "mc_plugin_base/mc_firmware/super_types.hpp"

#include <suspension_position_module_msgs/suspension_position_module_types.hpp>

namespace mcan {

namespace suspension_position_module {

// Hardware information
struct Hardware_t {
  static constexpr const char *k_name          = "suspension_position_module";
  static constexpr std::uint32_t k_time_stamp  = 1768431600;
  static constexpr std::uint32_t k_hw_revision = 1;
  static constexpr std::uint32_t k_fw_revision = 1;
  static constexpr std::uint64_t k_unique_id   = 0x1;
  static constexpr const char *k_description   = "Suspension module";
};

// Message definitions

namespace commands {

/**
 * @brief Set left offset
 * @details Command to set constant as offset
 * @units radians
 * @default 0.0
 * @permission rw
 */
struct SetEncoderOffsetLeft {
  using Type                                    = float;
  static constexpr const char *k_name           = "set_encoder_offset_left";
  static constexpr const char *k_group          = "commands";
  static constexpr std::uint32_t k_base_address = 0x010;
  static constexpr bool k_allow_read            = true;
  static constexpr bool k_allow_write           = true;

  float value = { 0.0 };
};

/**
 * @brief Set right offset
 * @details Command to set constant as offset
 * @units radians
 * @default 0.0
 * @permission rw
 */
struct SetEncoderOffsetRight {
  using Type                                    = float;
  static constexpr const char *k_name           = "set_encoder_offset_right";
  static constexpr const char *k_group          = "commands";
  static constexpr std::uint32_t k_base_address = 0x011;
  static constexpr bool k_allow_read            = true;
  static constexpr bool k_allow_write           = true;

  float value = { 0.0 };
};

} // namespace commands

namespace states {

/**
 * @details Current position of the suspension
 * @units radians
 * @default 0.0
 * @permission r
 */
struct CurrentPositionLeft {
  using Type                                    = float;
  static constexpr const char *k_name           = "current_position_left";
  static constexpr const char *k_group          = "states";
  static constexpr std::uint32_t k_base_address = 0x012;
  static constexpr bool k_allow_read            = true;
  static constexpr bool k_allow_write           = false;

  float value = { 0.0 };
};

/**
 * @details Current position of the suspension
 * @units radians
 * @default 0.0
 * @permission r
 */
struct CurrentPositionRight {
  using Type                                    = float;
  static constexpr const char *k_name           = "current_position_right";
  static constexpr const char *k_group          = "states";
  static constexpr std::uint32_t k_base_address = 0x013;
  static constexpr bool k_allow_read            = true;
  static constexpr bool k_allow_write           = false;

  float value = { 0.0 };
};

} // namespace states

namespace configs {

/**
 * @brief Discover Devices
 * @details This will trigger all devices on the bus to respond with their uniq ID that defines the hardware
 * @permission r
 */
struct DiscoverDevices {
  using Type                                    = uint64_t;
  static constexpr const char *k_name           = "discover_devices";
  static constexpr const char *k_group          = "configs";
  static constexpr std::uint32_t k_base_address = 0x001;
  static constexpr bool k_allow_read            = true;
  static constexpr bool k_allow_write           = false;

  uint64_t value;
};

/**
 * @brief Set Device Node ID
 * @details Set the device node ID of the module
 * @permission w
 */
struct SetDeviceNodeId {
  using Type                                    = uint8_t;
  static constexpr const char *k_name           = "set_device_node_id";
  static constexpr const char *k_group          = "configs";
  static constexpr std::uint32_t k_base_address = 0x002;
  static constexpr bool k_allow_read            = false;
  static constexpr bool k_allow_write           = true;

  uint8_t value;
};

/**
 * @brief Enter Configuration Mode
 * @details Command to enter configuration mode on all connected modules
 * @default 0
 * @permission w
 */
struct EnterConfigurationMode {
  using Type                                    = uint8_t;
  static constexpr const char *k_name           = "enter_configuration_mode";
  static constexpr const char *k_group          = "configs";
  static constexpr std::uint32_t k_base_address = 0x003;
  static constexpr bool k_allow_read            = false;
  static constexpr bool k_allow_write           = true;

  uint8_t value = { 0 };
};

/**
 * @details Device Identifier message
 * @permission r
 */
struct GetHardwareType {
  using Type                                    = HardwareType_t;
  static constexpr const char *k_name           = "get_hardware_type";
  static constexpr const char *k_group          = "configs";
  static constexpr std::uint32_t k_base_address = 0x004;
  static constexpr bool k_allow_read            = true;
  static constexpr bool k_allow_write           = false;

  HardwareType_t value;
};

/**
 * @brief Ping Module
 * @details Ping the module to check if it's alive, the counter will increment on each ping
 * @default 0
 * @permission w
 */
struct PingModule {
  using Type                                    = uint8_t;
  static constexpr const char *k_name           = "ping_module";
  static constexpr const char *k_group          = "configs";
  static constexpr std::uint32_t k_base_address = 0x005;
  static constexpr bool k_allow_read            = false;
  static constexpr bool k_allow_write           = true;

  uint8_t value = { 0 };
};

/**
 * @brief Flash Indicator LED
 * @details Command to flash the indicator LED on the module, useful for locating specific hardware
 * @default 0
 * @permission w
 */
struct FlashIndicatorLed {
  using Type                                    = bool;
  static constexpr const char *k_name           = "flash_indicator_led";
  static constexpr const char *k_group          = "configs";
  static constexpr std::uint32_t k_base_address = 0x006;
  static constexpr bool k_allow_read            = false;
  static constexpr bool k_allow_write           = true;

  bool value = { 0 };
};

} // namespace configs

// CAN MC Slave Interface
class McCanSlaveInterface_t {
public:
  commands::SetEncoderOffsetLeft set_encoder_offset_left;
  commands::SetEncoderOffsetRight set_encoder_offset_right;
  states::CurrentPositionLeft current_position_left;
  states::CurrentPositionRight current_position_right;

  auto get_write_variables() {
    return std::make_tuple(&McCanSlaveInterface_t::set_encoder_offset_left, &McCanSlaveInterface_t::set_encoder_offset_right);
  }

  auto get_read_variables() {
    return std::make_tuple(&McCanSlaveInterface_t::set_encoder_offset_left, &McCanSlaveInterface_t::set_encoder_offset_right,
                           &McCanSlaveInterface_t::current_position_left, &McCanSlaveInterface_t::current_position_right);
  }

  auto get_state_variables() {
    return std::make_tuple(&McCanSlaveInterface_t::current_position_left, &McCanSlaveInterface_t::current_position_right);
  }

  auto get_command_variables() {
    return std::make_tuple(&McCanSlaveInterface_t::set_encoder_offset_left, &McCanSlaveInterface_t::set_encoder_offset_right);
  }

  auto get_config_variables() {
    return std::make_tuple();
  }
};
} // namespace suspension_position_module

} // namespace mcan