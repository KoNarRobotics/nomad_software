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
#include "vesc_hardware/visiblity_control.h"

namespace nomad_hardware {

class MCVescHardware : public hardware_interface::SystemInterface {
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(MCVescHardware)

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

  HARDWARE_PUBLIC
  hardware_interface::return_type write(const rclcpp::Time &time, const rclcpp::Duration &period) override;

private:
  void can_callback_status_1(CanDriver &can_driver, const CanFrame &frame, void *args);
  void can_callback_status_2(CanDriver &can_driver, const CanFrame &frame, void *args);
  void can_callback_status_3(CanDriver &can_driver, const CanFrame &frame, void *args);
  void can_callback_status_4(CanDriver &can_driver, const CanFrame &frame, void *args);
  void can_callback_status_5(CanDriver &can_driver, const CanFrame &frame, void *args);
  void can_callback_status_6(CanDriver &can_driver, const CanFrame &frame, void *args);

  enum integration_level_t : std::uint8_t { POSITION = 1, VELOCITY = 2, CURRENT = 3 };

  struct joint_t {
    double position;
    double velocity;
    double current;

    double erpm;
    double duty_cycle;
    double amd_hours;
    double amd_hours_charged;
    double watt_hours;
    double watt_hours_charged;
    double temperature_mosfet;
    double temperature_motor;
    double current_in;
    double pid_pos;
    double tachometer;
    double voltage;

    double adc1;
    double adc2;
    double adc3;
    double ppm;
    integration_level_t integration_level;
  };

  std::vector<joint_t> joints;


  // Parameters for the RRBot simulation
  std::shared_ptr<CanDriver> can_driver;
};

} // namespace nomad_hardware
