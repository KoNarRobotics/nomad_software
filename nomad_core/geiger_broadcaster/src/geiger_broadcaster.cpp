// Copyright 2021 PAL Robotics SL.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * Authors: Subhas Das, Denis Stogl, Victor Lopez
 */

#include "geiger_broadcaster/geiger_broadcaster.hpp"

#include <memory>
#include <string>

using namespace nomad_core;


controller_interface::CallbackReturn GeigerBroadcaster::on_init() {
  try {
    param_listener_ = std::make_shared<ParamListener>(get_node());
    params_         = param_listener_->get_params();
  } catch(const std::exception &e) {
    RCLCPP_ERROR(get_node()->get_logger(), "Exception thrown during init stage with message: %s \n", e.what());
    return CallbackReturn::ERROR;
  }
  RCLCPP_INFO(get_node()->get_logger(), "GeigerBroadcaster initialized");
  return CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn GeigerBroadcaster::on_configure(const rclcpp_lifecycle::State & /*previous_state*/) {
  params_ = param_listener_->get_params();
  if(params_.sensor_name.empty()) {
    RCLCPP_ERROR(get_node()->get_logger(), "'sensor_name' parameter has to be specified.");
    return CallbackReturn::ERROR;
  }

  if(params_.frame_id.empty()) {
    RCLCPP_ERROR(get_node()->get_logger(), "'frame_id' parameter has to be provided.");
    return CallbackReturn::ERROR;
  }

  geiger_sensor_ = std::make_unique<GeigerSensor>(GeigerSensor(params_.sensor_name));

  try {
    // register ft sensor data publisher
    geiger_sensor_state_publisher_ =
    get_node()->create_publisher<geiger_msg::msg::Radiation>("~/radiation", rclcpp::SystemDefaultsQoS());
    realtime_radiation_publisher_ = std::make_unique<StatePublisherGeiger>(geiger_sensor_state_publisher_);

  } catch(const std::exception &e) {
    fprintf(stderr, "Exception thrown during publisher creation at configure stage with message : %s \n", e.what());
    return CallbackReturn::ERROR;
  }

  realtime_radiation_publisher_->lock();
  realtime_radiation_publisher_->msg_.header.frame_id = params_.frame_id;
  realtime_radiation_publisher_->unlock();

  RCLCPP_DEBUG(get_node()->get_logger(), "configure successful");
  return CallbackReturn::SUCCESS;
}

controller_interface::InterfaceConfiguration GeigerBroadcaster::command_interface_configuration() const {
  controller_interface::InterfaceConfiguration command_interfaces_config;
  command_interfaces_config.type = controller_interface::interface_configuration_type::NONE;
  return command_interfaces_config;
}

controller_interface::InterfaceConfiguration GeigerBroadcaster::state_interface_configuration() const {
  controller_interface::InterfaceConfiguration state_interfaces_config;
  state_interfaces_config.type  = controller_interface::interface_configuration_type::ALL;
  auto states                   = geiger_sensor_->get_state_interface_names();
  state_interfaces_config.names = states;
  return state_interfaces_config;
}

controller_interface::CallbackReturn GeigerBroadcaster::on_activate(const rclcpp_lifecycle::State & /*previous_state*/) {
  geiger_sensor_->assign_loaned_state_interfaces(state_interfaces_);
  return CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn GeigerBroadcaster::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/) {
  geiger_sensor_->release_interfaces();
  return CallbackReturn::SUCCESS;
}

controller_interface::return_type GeigerBroadcaster::update(const rclcpp::Time &time, const rclcpp::Duration & /*period*/) {
  if(realtime_radiation_publisher_ && realtime_radiation_publisher_->trylock()) {
    realtime_radiation_publisher_->msg_.header.stamp = time;
    geiger_sensor_->get_values_as_message(realtime_radiation_publisher_->msg_);
    realtime_radiation_publisher_->unlockAndPublish();
  }
  return controller_interface::return_type::OK;
}


#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nomad_core::GeigerBroadcaster, controller_interface::ControllerInterface)
