// Auto-generated plugin main file for ROS 2
//
// This will be converted to a plugin implementation
//

// ROS 2 includes
#include <rclcpp/rclcpp.hpp>

// Module type headers
#include <suspension_position_module_msgs/suspension_position_module_types.hpp>
#include <suspension_position_module_msgs/suspension_position_module.hpp>

// Conversion utilities
#include <suspension_position_module_msgs/conversions.hpp>

// Generated ROS messages
#include <suspension_position_module_msgs/msg/discover_devices.hpp>
#include <suspension_position_module_msgs/msg/set_device_node_id.hpp>
#include <suspension_position_module_msgs/msg/enter_configuration_mode.hpp>
#include <suspension_position_module_msgs/msg/get_hardware_type.hpp>
#include <suspension_position_module_msgs/msg/ping_module.hpp>
#include <suspension_position_module_msgs/msg/flash_indicator_led.hpp>
#include <suspension_position_module_msgs/msg/set_encoder_offset_left.hpp>
#include <suspension_position_module_msgs/msg/set_encoder_offset_right.hpp>
#include <suspension_position_module_msgs/msg/current_position_left.hpp>
#include <suspension_position_module_msgs/msg/current_position_right.hpp>

// Dependency conversions
#include <mcan_basic_module_msgs/conversions.hpp>
#include <mcan_ros_types_msgs/conversions.hpp>

#include <mc_plugin_driver/mc_plugin_driver.hpp>
#include <mc_plugin_driver/mc_slave_driver.hpp>
#include <mc_plugin_driver/mc_param.hpp>
#include <mc_plugin_driver/mc_plugin_exporter.hpp>
#include <mc_can_driver/can_linux_driver.hpp>


namespace mcan {
class suspension_position_module_plugin : public McPluginExporterBase {
public:
  suspension_position_module_plugin() : McPluginExporterBase() {
  }

  virtual Result<std::shared_ptr<McSlavePluginDriverBase>> create_new_instance(rclcpp::Node &node,
                                                                               std::shared_ptr<CanBase> can_primary,
                                                                               std::shared_ptr<CanBase> can_secondary,
                                                                               const ModuleParams &params) override {
    auto result =
    mcan::McSlavePluginDriver<mcan::suspension_position_module::McCanSlaveInterface_t, mcan::suspension_position_module::Hardware_t,
                              suspension_position_module_msgs::Conversions>::Make(node, std::move(can_primary),
                                                                                  std::move(can_secondary), params);

    if(!result.ok()) {
      return result.status();
    }
    return Result<std::shared_ptr<McSlavePluginDriverBase>>::OK(
    std::shared_ptr<McSlavePluginDriverBase>(result.valueOrDie()));
  }

  virtual uint64_t get_plugin_unique_id() const override {
    return mcan::suspension_position_module::Hardware_t::k_unique_id;
  };
};

}; // namespace mcan

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(mcan::suspension_position_module_plugin, mcan::McPluginExporterBase)
