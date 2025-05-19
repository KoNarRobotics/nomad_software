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

#include "controller_interface/controller_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"
#include "realtime_tools/realtime_publisher.hpp"
#include "semantic_components/semantic_component_interface.hpp"
#include "semantic_components/semantic_component_command_interface.hpp"
#include "sensor_msgs/msg/fluid_pressure.hpp"
#include "sensor_msgs/msg/temperature.hpp"

#include "semantic_components/range_sensor.hpp"
// #include "multi_interface_forward_command_controller.hpp"
#include "semantic_components/imu_sensor.hpp"
#include "barometer_broadcaster/barometer_broadcaster_parameters.hpp"

#include "barometer_broadcaster/barometer_sensor.hpp"

namespace nomad_core {
class BarometerBroadcaster : public controller_interface::ControllerInterface {
public:
  controller_interface::InterfaceConfiguration command_interface_configuration() const override;

  controller_interface::InterfaceConfiguration state_interface_configuration() const override;

  controller_interface::CallbackReturn on_init() override;

  controller_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state) override;

  controller_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state) override;

  controller_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  controller_interface::return_type update(const rclcpp::Time &time, const rclcpp::Duration &period) override;

protected:
  using ParamListener             = barometer_broadcaster::ParamListener;
  using Params                    = barometer_broadcaster::Params;
  using PressureSensor            = semantic_components::PressureSensor;
  using TemperatureSensor         = semantic_components::TemperatureSensor;
  using StatePublisherPressure    = realtime_tools::RealtimePublisher<sensor_msgs::msg::FluidPressure>;
  using StatePublisherTemperature = realtime_tools::RealtimePublisher<sensor_msgs::msg::Temperature>;

  // semantic_components::RangeSensor as;
  // controller_interface::
  std::shared_ptr<ParamListener> param_listener_;
  Params params_;

  std::unique_ptr<PressureSensor> pressure_sensor_;
  std::unique_ptr<TemperatureSensor> temperature_sensor_;


  rclcpp::Publisher<sensor_msgs::msg::FluidPressure>::SharedPtr barometer_sensor_state_publisher_;
  std::unique_ptr<StatePublisherPressure> realtime_pressure_publisher_;

  rclcpp::Publisher<sensor_msgs::msg::Temperature>::SharedPtr temperature_publisher_;
  std::unique_ptr<StatePublisherTemperature> realtime_temperature_publisher_;
};

} // namespace nomad_core
