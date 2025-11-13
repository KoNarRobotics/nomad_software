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

#include "imu_hardware/imu_hardware.hpp"


using namespace nomad_hardware;
using namespace nomad_core;

namespace {
constexpr const char *nameHardwareInterface = "MCImuHardware";
}

hardware_interface::CallbackReturn MCImuHardware::on_init(const hardware_interface::HardwareInfo &info) {
  if(hardware_interface::SensorInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "MCImuHardware creating...");

  auto mayby_can_interface = info_.hardware_parameters.find("can_interface");
  NOMAD_RC_RETURN_ON_ERROR_MSG(mayby_can_interface != info_.hardware_parameters.end(), nameHardwareInterface,
                               "No CAN interface specified");

  auto can_interface_name = mayby_can_interface->second;
  NOMAD_RC_RETURN_ON_ERROR_MSG(!can_interface_name.empty(), nameHardwareInterface, "CAN interface is empty");

  NOMAD_RC_ASSIGN_TO_OR_RETURN_MSG(can_driver, CanDriver::Make(can_interface_name), nameHardwareInterface,
                                   "Failed to create CAN driver");

  auto sensor_it =
  std::find_if(info_.sensors.begin(), info_.sensors.end(),
               [](const hardware_interface::ComponentInfo &sensor) { return sensor.name == "imu"; });
  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it != info_.sensors.end(), nameHardwareInterface, "IMU not found in hardware info");

  NOMAD_RC_RETURN_ON_ERROR_MSG(
  sensor_it->state_interfaces.size() == 14, nameHardwareInterface,
  "IMU should have 13 state interfaces: orientation.x, orientation.y, orientation.z, orientation.w, "
  "angular_velocity.x, angular_velocity.y, angular_velocity.z, linear_acceleration.x, "
  "linear_acceleration.y, linear_acceleration.z, magnetic_field.x, magnetic_field.y, magnetic_field.z, "
  "temperature");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCImuHardware::on_configure(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_IMU_STATUS_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_status, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_IMU_ORIENTATION_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_orientation,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_IMU_LINEAR_ACCELERATION_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_linear_acceleration,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_IMU_MAGNETIC_FIELD_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_magnetic_field,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_IMU_GYRATION_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_gyration, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_TEMPERATURE_IMU_STATUS_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_temperature_status,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_TEMPERATURE_IMU_TEMPERATURE_FRAME_ID,
                                                        std::bind(&MCImuHardware::can_callback_temperature_data,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCImuHardware::on_activate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "on_activate");
  imu_status                = 0;
  imu_orientation_x         = std::numeric_limits<double>::quiet_NaN();
  imu_orientation_y         = std::numeric_limits<double>::quiet_NaN();
  imu_orientation_z         = std::numeric_limits<double>::quiet_NaN();
  imu_orientation_w         = std::numeric_limits<double>::quiet_NaN();
  imu_linear_acceleration_x = std::numeric_limits<double>::quiet_NaN();
  imu_linear_acceleration_y = std::numeric_limits<double>::quiet_NaN();
  imu_linear_acceleration_z = std::numeric_limits<double>::quiet_NaN();
  imu_magnetic_field_x      = std::numeric_limits<double>::quiet_NaN();
  imu_magnetic_field_y      = std::numeric_limits<double>::quiet_NaN();
  imu_magnetic_field_z      = std::numeric_limits<double>::quiet_NaN();
  imu_gyration_x            = std::numeric_limits<double>::quiet_NaN();
  imu_gyration_y            = std::numeric_limits<double>::quiet_NaN();
  imu_gyration_z            = std::numeric_limits<double>::quiet_NaN();
  imu_temperature           = std::numeric_limits<double>::quiet_NaN();
  imu_temperature_status    = 0;

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->open_can(), nameHardwareInterface);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCImuHardware::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "on_deactivate");
  // (void)can_driver->remove_callback(CAN_IMU_STATUS_FRAME_ID);
  // (void)can_driver->remove_callback(CAN_IMU_ORIENTATION_FRAME_ID);
  // (void)can_driver->remove_callback(CAN_IMU_LINEAR_ACCELERATION_FRAME_ID);
  // (void)can_driver->remove_callback(CAN_IMU_MAGNETIC_FIELD_FRAME_ID);
  // (void)can_driver->remove_callback(CAN_IMU_GYRATION_FRAME_ID);
  // (void)can_driver->remove_callback(CAN_TEMPERATURE_IMU_STATUS_FRAME_ID);
  // (void)can_driver->remove_callback(CAN_TEMPERATURE_IMU_TEMPERATURE_FRAME_ID);

  NOMAD_RC_RETURN_ON_ERROR_NAM(CanDriver::close_can(can_driver), nameHardwareInterface);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type MCImuHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  (void)time;
  (void)period;
  // all frames are remote requests with size 0
  CanFrame frame;
  frame.size              = 0;
  frame.is_remote_request = true;


  frame.id          = CAN_IMU_STATUS_FRAME_ID;
  frame.is_extended = CAN_IMU_STATUS_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Status] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id          = CAN_IMU_ORIENTATION_FRAME_ID;
  frame.is_extended = CAN_IMU_ORIENTATION_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Orientation] CAN frame");
    return hardware_interface::return_type::ERROR;
  };


  frame.id          = CAN_IMU_LINEAR_ACCELERATION_FRAME_ID;
  frame.is_extended = CAN_IMU_LINEAR_ACCELERATION_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Linear Acceleration] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id          = CAN_IMU_MAGNETIC_FIELD_FRAME_ID;
  frame.is_extended = CAN_IMU_MAGNETIC_FIELD_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Magnetic Field] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id          = CAN_IMU_GYRATION_FRAME_ID;
  frame.is_extended = CAN_IMU_GYRATION_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Gyration] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id          = CAN_TEMPERATURE_IMU_STATUS_FRAME_ID;
  frame.is_extended = CAN_TEMPERATURE_IMU_STATUS_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Temperature Status] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id          = CAN_TEMPERATURE_IMU_TEMPERATURE_FRAME_ID;
  frame.is_extended = CAN_TEMPERATURE_IMU_TEMPERATURE_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Temperature Data] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  set_state("imu/orientation.x", imu_orientation_x);
  set_state("imu/orientation.y", imu_orientation_y);
  set_state("imu/orientation.z", imu_orientation_z);
  set_state("imu/orientation.w", imu_orientation_w);
  set_state("imu/linear_acceleration.x", imu_linear_acceleration_x);
  set_state("imu/linear_acceleration.y", imu_linear_acceleration_y);
  set_state("imu/linear_acceleration.z", imu_linear_acceleration_z);
  set_state("imu/magnetic_field.x", imu_magnetic_field_x);
  set_state("imu/magnetic_field.y", imu_magnetic_field_y);
  set_state("imu/magnetic_field.z", imu_magnetic_field_z);
  set_state("imu/angular_velocity.x", imu_gyration_x);
  set_state("imu/angular_velocity.y", imu_gyration_y);
  set_state("imu/angular_velocity.z", imu_gyration_z);
  set_state("imu/temperature", imu_temperature);

  return hardware_interface::return_type::OK;
}

void MCImuHardware::can_callback_status(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_imu_status_t status;
  if(can_imu_status_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu status");
    return;
  }
  imu_status = status.status;
}

void MCImuHardware::can_callback_orientation(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_imu_orientation_t orientation;
  if(can_imu_orientation_unpack(&orientation, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu orientation");
    return;
  }
  // double scale      = (1 << 14);
  // imu_orientation_x = (double)orientation.x / scale;
  // imu_orientation_y = (double)orientation.y / scale;
  // imu_orientation_z = (double)orientation.z / scale;
  // imu_orientation_w = (double)orientation.w / scale;
  imu_orientation_x = can_imu_orientation_x_decode(orientation.x);
  imu_orientation_y = can_imu_orientation_y_decode(orientation.y);
  imu_orientation_z = can_imu_orientation_z_decode(orientation.z);
  imu_orientation_w = can_imu_orientation_w_decode(orientation.w);
}

void MCImuHardware::can_callback_linear_acceleration(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_imu_linear_acceleration_t linear_acceleration;
  if(can_imu_linear_acceleration_unpack(&linear_acceleration, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu linear acceleration");
    return;
  }
  // double scale              = 100;
  // imu_linear_acceleration_x = (double)linear_acceleration.x / scale;
  // imu_linear_acceleration_y = (double)linear_acceleration.y / scale;
  // imu_linear_acceleration_z = (double)linear_acceleration.z / scale;
  imu_linear_acceleration_x = can_imu_linear_acceleration_x_decode(linear_acceleration.x);
  imu_linear_acceleration_y = can_imu_linear_acceleration_y_decode(linear_acceleration.y);
  imu_linear_acceleration_z = can_imu_linear_acceleration_z_decode(linear_acceleration.z);
}

void MCImuHardware::can_callback_magnetic_field(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_imu_magnetic_field_t magnetic_field;
  if(can_imu_magnetic_field_unpack(&magnetic_field, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu magnetic field");
    return;
  }
  // double scale         = 16;
  // imu_magnetic_field_x = (double)magnetic_field.x / scale;
  // imu_magnetic_field_y = (double)magnetic_field.y / scale;
  // imu_magnetic_field_z = (double)magnetic_field.z / scale;

  imu_magnetic_field_x = can_imu_magnetic_field_x_decode(magnetic_field.x);
  imu_magnetic_field_y = can_imu_magnetic_field_y_decode(magnetic_field.y);
  imu_magnetic_field_z = can_imu_magnetic_field_z_decode(magnetic_field.z);
}

void MCImuHardware::can_callback_gyration(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_imu_gyration_t gyration;
  if(can_imu_gyration_unpack(&gyration, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu gyration");
    return;
  }
  // double scale   = 900; // 100
  // imu_gyration_x = (double)gyration.x / scale;
  // imu_gyration_y = (double)gyration.y / scale;
  // imu_gyration_z = (double)gyration.z / scale;
  imu_gyration_x = can_imu_gyration_x_decode(gyration.x);
  imu_gyration_y = can_imu_gyration_y_decode(gyration.y);
  imu_gyration_z = can_imu_gyration_z_decode(gyration.z);
}

void MCImuHardware::can_callback_temperature_status(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_temperature_imu_status_t temperature_status;
  if(can_temperature_imu_status_unpack(&temperature_status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu temperature status");
    return;
  }
  imu_temperature_status = temperature_status.accelerometer;
}


void MCImuHardware::can_callback_temperature_data(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_temperature_imu_temperature_t temperature_data;
  if(can_temperature_imu_temperature_unpack(&temperature_data, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack imu temperature data");
    return;
  }
  // for temp scale is 1
  imu_temperature = temperature_data.accelerometer;
}

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nomad_hardware::MCImuHardware, hardware_interface::SensorInterface)