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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/actuator_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/sensor_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "can_device/can_device.hpp"
#include "imu_hardware/visiblity_control.h"

namespace nomad_hardware {

class MCImuHardware : public hardware_interface::SensorInterface {
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(MCImuHardware)

  HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo &info) override;

  HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state) override;

  HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state) override;

  HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  HARDWARE_PUBLIC
  hardware_interface::return_type read(const rclcpp::Time &time, const rclcpp::Duration &period) override;

private:
  void can_callback_status(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_orientation(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_linear_acceleration(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_magnetic_field(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_gyration(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_temperature_status(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_temperature_data(CanDriver &can, const CanFrame &frame, void *args);


  double imu_orientation_x;
  double imu_orientation_y;
  double imu_orientation_z;
  double imu_orientation_w;

  double imu_linear_acceleration_x;
  double imu_linear_acceleration_y;
  double imu_linear_acceleration_z;

  double imu_magnetic_field_x;
  double imu_magnetic_field_y;
  double imu_magnetic_field_z;

  double imu_gyration_x;
  double imu_gyration_y;
  double imu_gyration_z;

  double imu_temperature;

  int imu_status;
  int imu_temperature_status;

  std::shared_ptr<CanDriver> can_driver;
};

} // namespace nomad_hardware