
#pragma once

#include <limits>
#include <string>
#include <vector>

#include "semantic_components/semantic_component_interface.hpp"
#include "sensor_msgs/msg/fluid_pressure.hpp"
#include "sensor_msgs/msg/temperature.hpp"

namespace semantic_components {

class PressureSensor : public SemanticComponentInterface<sensor_msgs::msg::FluidPressure> {
public:
  explicit PressureSensor(const std::string &name)
  : SemanticComponentInterface(name, { name + "/" + "pressure" }) {
  }
  /**
   * Return Range reported by a sensor
   *
   * \return value of the range in meters
   */
  float get_pressure() const {
    const auto data = state_interfaces_[0].get().get_optional();
    // return data;
    if(data.has_value()) {
      return data.value();
    }
    return std::numeric_limits<float>::quiet_NaN();
  }

  /// Return Range message with range in meters
  /**
   * Constructs and return a Range message from the current values.
   * \return Range message from values;
   */
  bool get_values_as_message(sensor_msgs::msg::FluidPressure &message) const {
    message.fluid_pressure = get_pressure();
    message.variance       = 0.0;
    return true;
  }
};

class TemperatureSensor : public SemanticComponentInterface<sensor_msgs::msg::Temperature> {
public:
  explicit TemperatureSensor(const std::string &name)
  : SemanticComponentInterface(name, { name + "/" + "temperature" }) {
  }

  float get_temperature() const {
    const auto data = state_interfaces_[0].get().get_optional();
    if(data.has_value()) {
      return data.value();
    }
    return std::numeric_limits<float>::quiet_NaN();
  }

  bool get_values_as_message(sensor_msgs::msg::Temperature &message) const {
    message.temperature = get_temperature();
    message.variance    = 0.0;
    return true;
  }
};

} // namespace semantic_components
