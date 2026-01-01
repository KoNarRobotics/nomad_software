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

#include "geiger_hardware/geiger_hardware.hpp"


using namespace nomad_hardware;

namespace {
constexpr const char *nameHardwareInterface = "MCGeigerHardware";
} // namespace

#define VESC_ID_BASE_START 0x14

hardware_interface::CallbackReturn MCGeigerHardware::on_init(const hardware_interface::HardwareInfo &info) {
  if(hardware_interface::SensorInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  prefix = info_.hardware_parameters["prefix"];
  if(!prefix.empty() && prefix.back() != '/') {
    prefix += '/';
  }

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Sensor prefix: %s", prefix.c_str());
  // can_interface_name is the name of the can interface we will use to communicate with the VESC

  auto mayby_can_interface = info_.hardware_parameters.find("can_interface");
  NOMAD_RC_RETURN_ON_ERROR_MSG(mayby_can_interface != info_.hardware_parameters.end(), nameHardwareInterface,
                               "No CAN interface specified");

  auto can_interface_name = mayby_can_interface->second;


  auto sensor_it =
  std::find_if(info_.sensors.begin(), info_.sensors.end(),
               [](const hardware_interface::ComponentInfo &sensor) { return sensor.name == "geiger"; });
  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it != info_.sensors.end(), nameHardwareInterface, "\"geiger\" not found in hardware info");

  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it->state_interfaces.size() == 2, nameHardwareInterface,
                               "\"geiger\" should have 2 state interfaces: cpm, siwert");

  NOMAD_RC_ASSIGN_TO_OR_RETURN_MSG(can_driver, CanDriver::Make(can_interface_name), nameHardwareInterface,
                                   "Failed to create CAN driver");

  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "CAN driver created on interface %s",
               can_interface_name.c_str());


  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGeigerHardware::on_configure(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;

  for(const auto &[name, descr] : sensor_state_interfaces_) {
    set_state(name, 0.0);
    RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Configuring interface: %s!", name.c_str());
  }

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GEIGER_READ_FRAME_ID,
                                                        std::bind(&MCGeigerHardware::can_callback_geiger_read,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3)),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GEIGER_STATUS_FRAME_ID,
                                                        std::bind(&MCGeigerHardware::can_callback_status, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3)),
                               nameHardwareInterface);


  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Successfully configured!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGeigerHardware::on_activate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->open_can(), nameHardwareInterface);
  geiger_status = CAN_GEIGER_STATUS_GEIGER_STATUS_DISCONNECTED_CHOICE;

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Sensor successfully activated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGeigerHardware::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(CanDriver::close_can(can_driver), nameHardwareInterface);

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Sensor successfully deactivated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type MCGeigerHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  (void)time;
  (void)period;

  CanFrame frame;
  frame.is_remote_request = true;
  frame.id                = CAN_GEIGER_STATUS_FRAME_ID;
  frame.size              = CAN_GEIGER_STATUS_LENGTH;
  frame.is_extended       = CAN_GEIGER_READ_IS_EXTENDED;
  auto status             = can_driver->send(frame);
  if(!status.ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [geiger_status] CAN frame | %s",
                 status.to_string().c_str());
    return hardware_interface::return_type::ERROR;
  };


  frame.is_remote_request = true;
  frame.id                = CAN_GEIGER_READ_FRAME_ID;
  frame.size              = CAN_GEIGER_READ_LENGTH;
  frame.is_extended       = CAN_GEIGER_READ_IS_EXTENDED;
  status                  = can_driver->send(frame);
  if(!status.ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [geiger_read] CAN frame | %s",
                 status.to_string().c_str());
    return hardware_interface::return_type::ERROR;
  };

  double _cpm    = cpm;
  double _siwert = siwert;
  switch(geiger_status) {
  case CAN_GEIGER_STATUS_GEIGER_STATUS_OK_CHOICE: break;
  case CAN_GEIGER_STATUS_GEIGER_STATUS_DISCONNECTED_CHOICE:
    _cpm    = std::numeric_limits<double>::quiet_NaN();
    _siwert = std::numeric_limits<double>::quiet_NaN();
    RCLCPP_WARN_STREAM(rclcpp::get_logger(nameHardwareInterface), "Geiger sensor disconnected!");
    break;
  case CAN_GEIGER_STATUS_GEIGER_STATUS_FAULT_CHOICE:
    _cpm    = std::numeric_limits<double>::quiet_NaN();
    _siwert = std::numeric_limits<double>::quiet_NaN();
    RCLCPP_ERROR_STREAM(rclcpp::get_logger(nameHardwareInterface), "Geiger sensor failure!");
    break;
  default: break;
  }
  set_state(prefix + "geiger/cpm", _cpm);
  set_state(prefix + "geiger/siwert", _siwert);
  return hardware_interface::return_type::OK;
}

void MCGeigerHardware::can_callback_status(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;
  can_geiger_status_t msg;
  if(can_geiger_status_unpack(&msg, frame.data, frame.size) != 0) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger(nameHardwareInterface),
                        "Geiger sensor failed to unpack \"geiger_status\"!");
    return;
  }
  geiger_status = msg.geiger_status;
}


void MCGeigerHardware::can_callback_geiger_read(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;
  can_geiger_read_t msg;
  if(can_geiger_read_unpack(&msg, frame.data, frame.size) != 0) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger(nameHardwareInterface),
                        "Geiger sensor failed to unpack \"geiger_read\"!");
    return;
  }
  cpm    = msg.cpm;
  siwert = msg.micro_siwert * 0.000001;
}


#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nomad_hardware::MCGeigerHardware, hardware_interface::SensorInterface)