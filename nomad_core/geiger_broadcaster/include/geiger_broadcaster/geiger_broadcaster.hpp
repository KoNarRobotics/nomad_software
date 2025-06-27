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
#include "geiger_msg/msg/radiation.hpp"


#include "geiger_broadcaster/geiger_broadcaster_parameters.hpp"

#include "geiger_broadcaster/geiger_sensor.hpp"

namespace nomad_core {
class GeigerBroadcaster : public controller_interface::ControllerInterface {
public:
  controller_interface::InterfaceConfiguration command_interface_configuration() const override;

  controller_interface::InterfaceConfiguration state_interface_configuration() const override;

  controller_interface::CallbackReturn on_init() override;

  controller_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State &previous_state) override;

  controller_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state) override;

  controller_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  controller_interface::return_type update(const rclcpp::Time &time, const rclcpp::Duration &period) override;

protected:
  using ParamListener        = geiger_broadcaster::ParamListener;
  using Params               = geiger_broadcaster::Params;
  using GeigerSensor         = semantic_components::GeigerSensor;
  using StatePublisherGeiger = realtime_tools::RealtimePublisher<geiger_msg::msg::Radiation>;

  std::shared_ptr<ParamListener> param_listener_;
  Params params_;

  std::unique_ptr<GeigerSensor> geiger_sensor_;


  rclcpp::Publisher<geiger_msg::msg::Radiation>::SharedPtr geiger_sensor_state_publisher_;
  std::unique_ptr<StatePublisherGeiger> realtime_radiation_publisher_;
};

} // namespace nomad_core
