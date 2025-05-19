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

#include "barometer_hardware/barometer_hardware.hpp"


using namespace nomad_hardware;
using namespace nomad_core;

namespace {
constexpr const char *nameHardwareInterface = "MCBarometerHardware";
}

hardware_interface::CallbackReturn MCBarometerHardware::on_init(const hardware_interface::HardwareInfo &info) {
  if(hardware_interface::SensorInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  auto it = info.hardware_parameters.find("can_interface");

  NOMAD_RC_RETURN_ON_ERROR_MSG(it == info.hardware_parameters.end(), nameHardwareInterface, "No CAN interface specified");

  auto can_interface_name = it->second;
  NOMAD_RC_RETURN_ON_ERROR_MSG(can_interface_name.empty(), nameHardwareInterface, "CAN interface is empty");

  NOMAD_RC_ASSIGN_TO_OR_RETURN_MSG(can_driver, CanDriver::Make(can_interface_name, false, 1000, 32),
                                   nameHardwareInterface, "Failed to create CAN driver");


  auto sensor_it =
  std::find_if(info_.sensors.begin(), info_.sensors.end(),
               [](const hardware_interface::ComponentInfo &sensor) { return sensor.name == "barometer"; });

  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it == info_.sensors.end(), nameHardwareInterface, "Barometer not found in hardware info");

  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it->state_interfaces.size() == 2, nameHardwareInterface,
                               "Barometer should have 2 state interfaces");
  // auto sensor_index = std::distance(info_.sensors.begin(), sensor_it);

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCBarometerHardware::on_configure(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_BAROMETER_STATUS_FRAME_ID,
                                                        std::bind(&MCBarometerHardware::can_callback_status,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_BAROMETER_DATA_FRAME_ID,
                                                        std::bind(&MCBarometerHardware::can_callback_data,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  for(const auto &[name, descr] : sensor_state_interfaces_) {
    set_state(name, 0.0);
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCBarometerHardware::on_activate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "on_activate");
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->open_can(), nameHardwareInterface);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCBarometerHardware::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "on_deactivate");
  NOMAD_RC_RETURN_ON_ERROR_NAM(CanDriver::close_can(can_driver), nameHardwareInterface);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type MCBarometerHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  (void)time;
  (void)period;
  CanFrame frame;
  frame.id                = CAN_BAROMETER_STATUS_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_BAROMETER_DATA_IS_EXTENDED;
  can_driver->send(frame);
  if(!can_driver->receive().ok())
    return hardware_interface::return_type::ERROR;
  frame.id                = CAN_BAROMETER_DATA_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_BAROMETER_DATA_IS_EXTENDED;
  can_driver->send(frame);
  can_driver->receive();
  if(!can_driver->receive().ok())
    return hardware_interface::return_type::ERROR;

  for(const auto &[name, descr] : sensor_state_interfaces_) {
    RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Sensor: %s  Desc %s", name.c_str(),
                descr.interface_name.c_str());
  }


  return hardware_interface::return_type::OK;
}

void MCBarometerHardware::can_callback_status(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_barometer_status_t status;
  if(can_barometer_status_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack barometer status");
    return;
  }
  barometer_status = status.status;
}

void MCBarometerHardware::can_callback_data(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_barometer_data_t data;
  if(can_barometer_data_unpack(&data, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack barometer data");
    return;
  }
  barometer_temperature = data.temperature;
  barometer_pressure    = data.pressure;
}

// hardware_interface::return_type MCBarometerHardware::write(const rclcpp::Time &time, const rclcpp::Duration &period) {
//   // Write data to the barometer if needed
//   return hardware_interface::return_type::OK;
// }
