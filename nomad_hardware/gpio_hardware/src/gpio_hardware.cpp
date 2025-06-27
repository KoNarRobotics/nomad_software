/*
Copyright (c) 2025 Patryk Dudziński

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Authors: Patryk Dudziński
 */

#include "hardware_interface/actuator_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "can_device/can_device.hpp"
#include "can_device/can_messages.h"

#include "gpio_hardware/gpio_hardware.hpp"


using namespace nomad_hardware;
using namespace nomad_core;

namespace {
constexpr const char *nameHardwareInterface = "MCGpioHardware";
} // namespace

#define VESC_ID_BASE_START 0x14

hardware_interface::CallbackReturn MCGpioHardware::on_init(const hardware_interface::HardwareInfo &info) {
  if(hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  prefix = info_.hardware_parameters["prefix"];
  // can_interface_name is the name of the can interface we will use to communicate with the VESC
  auto can_interface_name = info_.hardware_parameters["can_interface"];
  // gpio amount
  auto mayby_gpio_count = info_.hardware_parameters["gpio_count"];

  // Set polar_pairs based on mayby_polar_pairs
  if(mayby_gpio_count.empty()) {
    gpio_count = 4;
    RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Using default gpio count: %i", gpio_count);
  } else {
    try {
      gpio_count = std::stoi(mayby_gpio_count);
      RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Using gpio count: %i", gpio_count);
    } catch(const std::exception &e) {
      RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to parse gpio count: %s", e.what());
      return hardware_interface::CallbackReturn::ERROR;
    }
  }


  NOMAD_RC_ASSIGN_TO_OR_RETURN_MSG(can_driver, CanDriver::Make(can_interface_name), nameHardwareInterface,
                                   "Failed to create CAN driver");


  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "CAN driver created on interface %s",
               can_interface_name.c_str());


  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGpioHardware::on_configure(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;

  for(const auto &[name, descr] : joint_state_interfaces_) {
    set_state(name, 0.0);
  }
  for(const auto &[name, descr] : joint_command_interfaces_) {
    set_command(name, 0.0);
  }


  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Successfully configured!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGpioHardware::on_activate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->open_can(), nameHardwareInterface);

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "System successfully activated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGpioHardware::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(CanDriver::close_can(can_driver), nameHardwareInterface);

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "System successfully deactivated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type MCGpioHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  // nothing to do here sinc vesc sends the data at random intervals
  // joint are in order
  // FLEFT  front left
  // FRIGHT front right
  // RLEFT  rear left
  // RRIGHT rear right
  (void)time;
  (void)period;
  // set_state(prefix + "left_wheel_joint/position", joints[0].position);

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type MCGpioHardware::write(const rclcpp::Time &time, const rclcpp::Duration &period) {
  (void)time;
  (void)period;

  return hardware_interface::return_type::OK;
}

void MCGpioHardware::can_callback_status(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
}

void MCGpioHardware::can_callback_gpio_read(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
}


#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nomad_hardware::MCGpioHardware, hardware_interface::SystemInterface)