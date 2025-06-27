
#pragma once

#include <limits>
#include <string>
#include <vector>

#include "semantic_components/semantic_component_interface.hpp"
#include "sensor_msgs/msg/fluid_pressure.hpp"
#include "sensor_msgs/msg/temperature.hpp"
#include "geiger_msg/msg/radiation.hpp"

namespace semantic_components {

class GeigerSensor : public SemanticComponentInterface<geiger_msg::msg::Radiation> {
public:
  explicit GeigerSensor(const std::string &name)
  : SemanticComponentInterface(name, { name + "/siwert", name + "/cpm" }) {
  }
  /**
   * Return Range reported by a sensor
   *
   * \return value of the range in meters
   */
  float get_siwert() const {
    const auto data = state_interfaces_[0].get().get_optional();
    if(data.has_value()) {
      return data.value();
    }
    return std::numeric_limits<float>::quiet_NaN();
  }

  float get_cpm() const {
    const auto data = state_interfaces_[1].get().get_optional();
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
  bool get_values_as_message(geiger_msg::msg::Radiation &message) const {
    message.siwert = get_siwert();
    message.cpm    = get_cpm();
    return true;
  }
};

} // namespace semantic_components
