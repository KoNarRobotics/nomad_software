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
#include "gps_hardware/visiblity_control.h"
#include <ctime>

namespace nomad_hardware {

enum class MCGpsHardwareStatus : uint8_t {
  OK           = CAN_GPS_STATUS_STATUS_OK_CHOICE,
  DISCONNECTED = CAN_GPS_STATUS_STATUS_DISCONNECTED_CHOICE,
  ERROR        = CAN_GPS_STATUS_STATUS_ERROR_CHOICE
};

enum class MCGpsSignalType : int8_t {
  UNABLE_TO_FIX_POSITION       = CAN_GPS_STATUS_SIGNAL_UNABLE_TO_FIX_POSITION_CHOICE,
  UNAUGMENTED_FIX              = CAN_GPS_STATUS_SIGNAL_UNAUGMENTED_FIX_CHOICE,
  SATELLITE_BASED_AUGMENTATION = CAN_GPS_STATUS_SIGNAL_SATELLITE_BASED_AUGMENTATION_CHOICE,
  GROUND_BASED_AUGMENTATION    = CAN_GPS_STATUS_SIGNAL_GROUND_BASED_AUGMENTATION_CHOICE,
};

class MCGpsHardware : public hardware_interface::SensorInterface {
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(MCGpsHardware)

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

  // HARDWARE_PUBLIC
  // hardware_interface::return_type write(const rclcpp::Time &time, const rclcpp::Duration &period) override;

private:
  void can_callback_status(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_latitude(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_longitude(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_altitude(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_date(CanDriver &can, const CanFrame &frame, void *args);
  void can_callback_covariance(CanDriver &can, const CanFrame &frame, void *args);

  MCGpsHardwareStatus status;
  MCGpsSignalType signal_type;
  double latitude;
  double longitude;
  double altitude;
  double latitude_covariance;
  double longitude_covariance;
  double altitude_covariance;
  std::tm date_time;


  std::shared_ptr<CanDriver> can_driver;
};

} // namespace nomad_hardware