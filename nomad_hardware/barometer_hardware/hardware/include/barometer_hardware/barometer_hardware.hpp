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
#include "barometer_hardware/visiblity_control.h"

namespace nomad_hardware {

class MCBarometerHardware : public hardware_interface::SensorInterface {
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(MCBarometerHardware)

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
  void can_callback_data(CanDriver &can, const CanFrame &frame, void *args);


  float barometer_pressure;
  float barometer_temperature;
  int barometer_status;
  std::unordered_map<std::string, float *> barometer_state_map;

  // Parameters for the RRBot simulation
  std::shared_ptr<CanDriver> can_driver;
};

} // namespace nomad_hardware