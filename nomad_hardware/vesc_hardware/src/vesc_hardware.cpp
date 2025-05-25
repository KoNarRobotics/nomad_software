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

#include "hardware_interface/actuator_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "can_device/can_device.hpp"
#include "can_device/can_messages.h"

#include "vesc_hardware/vesc_hardware.hpp"


using namespace nomad_hardware;
using namespace nomad_core;

namespace {
constexpr const char *nameHardwareInterface = "MCVescHardware";
static const double current_to_torque       = 0.0665; // 66.5 mNm/A is the torque constant for the VESC motor
} // namespace

#define VESC_ID_BASE_START 0x14

hardware_interface::CallbackReturn MCVescHardware::on_init(const hardware_interface::HardwareInfo &info) {
  if(hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  auto can_interface_name = info_.hardware_parameters["can_interface"];
  prefix                  = info_.hardware_parameters["prefix"];

  NOMAD_RC_ASSIGN_TO_OR_RETURN_MSG(can_driver, CanDriver::Make(can_interface_name), nameHardwareInterface,
                                   "Failed to create CAN driver");

  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "CAN driver created on interface %s",
               can_interface_name.c_str());

  if(info_.joints.size() != 4) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "VESC hardware should have 4 joints");
    return hardware_interface::CallbackReturn::ERROR;
  }

  for(const hardware_interface::ComponentInfo &joint : info_.joints) {
    if(joint.command_interfaces.size() != 3) {
      RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "VESC joint should have 3 command interfaces");
      return hardware_interface::CallbackReturn::ERROR;
    }

    if(!(joint.command_interfaces[0].name == hardware_interface::HW_IF_VELOCITY ||
         joint.command_interfaces[0].name == hardware_interface::HW_IF_CURRENT ||
         joint.command_interfaces[0].name == hardware_interface::HW_IF_EFFORT)) {
      RCLCPP_FATAL(get_logger(), "Joint '%s' has %s command interface. Expected %s, %s or %s", joint.name.c_str(),
                   joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_VELOCITY,
                   hardware_interface::HW_IF_CURRENT, hardware_interface::HW_IF_EFFORT);
      return hardware_interface::CallbackReturn::ERROR;
    }

    if(joint.state_interfaces.size() != 4) {
      RCLCPP_FATAL(get_logger(), "Joint '%s'has %zu state interfaces. 4 expected.", joint.name.c_str(),
                   joint.command_interfaces.size());
      return hardware_interface::CallbackReturn::ERROR;
    }

    if(!(joint.state_interfaces[0].name == hardware_interface::HW_IF_POSITION ||
         joint.state_interfaces[0].name == hardware_interface::HW_IF_VELOCITY ||
         joint.state_interfaces[0].name == hardware_interface::HW_IF_CURRENT ||
         joint.state_interfaces[0].name == hardware_interface::HW_IF_EFFORT)) {
      RCLCPP_FATAL(get_logger(), "Joint '%s' has %s state interface. Expected %s, %s, or %s.",
                   joint.name.c_str(), joint.state_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION,
                   hardware_interface::HW_IF_VELOCITY, hardware_interface::HW_IF_CURRENT);
      return hardware_interface::CallbackReturn::ERROR;
    }
  }

  joints.resize(4);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCVescHardware::on_configure(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;

  for(const auto &[name, descr] : joint_state_interfaces_) {
    set_state(name, 0.0);
  }
  for(const auto &[name, descr] : joint_command_interfaces_) {
    set_command(name, 0.0);
  }

  for(size_t i = 0; i < 4; i++) {
    // thiss will genrate the frame ids for the 4 motors statring with 4 ending at 7
    uint32_t st1_id = (CAN_VESC_FLEFT_STATUS_1_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
    uint32_t st2_id = (CAN_VESC_FLEFT_STATUS_2_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
    uint32_t st3_id = (CAN_VESC_FLEFT_STATUS_3_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
    uint32_t st4_id = (CAN_VESC_FLEFT_STATUS_4_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
    uint32_t st5_id = (CAN_VESC_FLEFT_STATUS_5_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
    uint32_t st6_id = (CAN_VESC_FLEFT_STATUS_6_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);

    NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(st1_id,
                                                          std::bind(&MCVescHardware::can_callback_status_1,
                                                                    this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3),
                                                          &joints[i]),
                                 nameHardwareInterface);

    NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(st2_id,
                                                          std::bind(&MCVescHardware::can_callback_status_2,
                                                                    this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3),
                                                          &joints[i]),
                                 nameHardwareInterface);

    NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(st3_id,
                                                          std::bind(&MCVescHardware::can_callback_status_3,
                                                                    this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3),
                                                          &joints[i]),
                                 nameHardwareInterface);

    NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(st4_id,
                                                          std::bind(&MCVescHardware::can_callback_status_4,
                                                                    this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3),
                                                          &joints[i]),
                                 nameHardwareInterface);

    NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(st5_id,
                                                          std::bind(&MCVescHardware::can_callback_status_5,
                                                                    this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3),
                                                          &joints[i]),
                                 nameHardwareInterface);

    NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(st6_id,
                                                          std::bind(&MCVescHardware::can_callback_status_6,
                                                                    this, std::placeholders::_1,
                                                                    std::placeholders::_2, std::placeholders::_3),
                                                          &joints[i]),
                                 nameHardwareInterface);
  }

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Successfully configured!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCVescHardware::on_activate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->open_can(), nameHardwareInterface);

  for(size_t i = 0; i < joints.size(); i++) {
    joints[i]                   = {};
    joints[i].integration_level = integration_level_t::VELOCITY;
  }

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "System successfully activated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCVescHardware::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(CanDriver::close_can(can_driver), nameHardwareInterface);

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "System successfully deactivated!");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type MCVescHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  // nothing to do here sinc vesc sends the data at random intervals
  // joint are in order
  // FLEFT  front left
  // FRIGHT front right
  // RLEFT  rear left
  // RRIGHT rear right
  (void)time;
  (void)period;
  set_state(prefix + "left_wheel_joint/position", joints[0].position);
  set_state(prefix + "left_wheel_joint/velocity", joints[0].velocity);
  set_state(prefix + "left_wheel_joint/current", joints[0].current);
  set_state(prefix + "left_wheel_joint/effort", joints[0].current * current_to_torque);

  set_state(prefix + "right_wheel_joint/position", joints[1].position);
  set_state(prefix + "right_wheel_joint/velocity", joints[1].velocity);
  set_state(prefix + "right_wheel_joint/current", joints[1].current);
  set_state(prefix + "right_wheel_joint/effort", joints[1].current * current_to_torque);

  set_state(prefix + "left_wheel_joint_2/position", joints[2].position);
  set_state(prefix + "left_wheel_joint_2/velocity", joints[2].velocity);
  set_state(prefix + "left_wheel_joint_2/current", joints[2].current);
  set_state(prefix + "left_wheel_joint_2/effort", joints[2].current * current_to_torque);


  set_state(prefix + "right_wheel_joint_2/position", joints[3].position);
  set_state(prefix + "right_wheel_joint_2/velocity", joints[3].velocity);
  set_state(prefix + "right_wheel_joint_2/current", joints[3].current);
  set_state(prefix + "right_wheel_joint_2/effort", joints[3].current * current_to_torque);
  // for(size_t i = 0; i < joints.size(); ++i) {
  //   RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Joint %zu: position = %f, velocity = %f", i,
  //               joints[i].position, joints[i].velocity);
  // }
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type MCVescHardware::write(const rclcpp::Time &time, const rclcpp::Duration &period) {
  (void)time;
  (void)period;
  // joint are in order
  // FLEFT  front left
  // FRIGHT front right
  // RLEFT  rear left
  // RRIGHT rear right
  std::vector<double> joint_cur{ get_command<double>(prefix + "left_wheel_joint/current"),
                                 get_command<double>(prefix + "right_wheel_joint/current"),
                                 get_command<double>(prefix + "left_wheel_joint_2/current"),
                                 get_command<double>(prefix + "right_wheel_joint_2/current") };
  std::vector<double> joint_vel{ get_command<double>(prefix + "left_wheel_joint/velocity"),
                                 get_command<double>(prefix + "right_wheel_joint/velocity"),
                                 get_command<double>(prefix + "left_wheel_joint_2/velocity"),
                                 get_command<double>(prefix + "right_wheel_joint_2/velocity") };
  std::vector<double> joint_effort{ get_command<double>(prefix + "left_wheel_joint/effort"),
                                    get_command<double>(prefix + "right_wheel_joint/effort"),
                                    get_command<double>(prefix + "left_wheel_joint_2/effort"),
                                    get_command<double>(prefix + "right_wheel_joint_2/effort") };
  std::vector<double> joint_pos{ 0.0, 0.0, 0.0, 0.0 };

  // auto left_velocity = get_command<double>(prefix + "left_wheel_joint/velocity");
  // auto left_current  = get_command<double>(prefix + "left_wheel_joint/current");
  // auto left_effort   = get_command<double>(prefix + "left_wheel_joint/effort");

  // auto right_velocity = get_command<double>(prefix + "right_wheel_joint/velocity");
  // auto right_current  = get_command<double>(prefix + "right_wheel_joint/current");
  // auto right_effort   = get_command<double>(prefix + "right_wheel_joint/effort");

  // auto left_velocity_2 = get_command<double>(prefix + "left_wheel_joint_2/velocity");
  // auto left_current_2  = get_command<double>(prefix + "left_wheel_joint_2/current");
  // auto left_effort_2   = get_command<double>(prefix + "left_wheel_joint_2/effort");

  // auto right_velocity_2 = get_command<double>(prefix + "right_wheel_joint_2/velocity");
  // auto right_current_2  = get_command<double>(prefix + "right_wheel_joint_2/current");
  // auto right_effort_2   = get_command<double>(prefix + "right_wheel_joint_2/effort");


  // Value given by ros2 are usual physical values, but the vesc expects them boring ones so.
  // for current we get amps so we are good
  // for position we get rad so we need to convert it to degrees but its doesn;t need postion so we ignore it

  // std::vector<double> joint_cur    = { left_current, right_current, left_current_2, right_current_2 };
  // std::vector<double> joint_vel    = { left_velocity, right_velocity, left_velocity_2, right_velocity_2 };
  // std::vector<double> joint_pos    = { 0.0, 0.0, 0.0, 0.0 };
  // std::vector<double> joint_effort = { left_effort, right_effort, left_effort_2, right_effort_2 };
  // RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Write lv: %f  rv: %f", left_velocity, right_velocity);

  for(size_t i = 0; i < 4; i++) {
    joint_t &joint = joints[i];
    CanFrame frame;
    switch(joint.integration_level) {
    case integration_level_t::CURRENT: {
      can_vesc_fleft_set_current_t str;
      double current = joint_cur[i] * 1000.0; // the value of current is in mA | so we need to multiply it by 1000
      str.current = (uint32_t)current;
      frame.id    = (CAN_VESC_FLEFT_SET_CURRENT_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
      frame.size  = CAN_VESC_FLEFT_SET_CURRENT_LENGTH;
      can_vesc_fleft_set_current_pack(frame.data, &str, frame.size);
      break;
    }
    case integration_level_t::VELOCITY: {
      can_vesc_fleft_set_rpm_t str;
      // for velocity we get rad/s so we need to convert it to rpm
      // 1 rad/s = 60 / (2 * PI) rpm

      double rpm = joint_vel[i] * 60.0 / (2.0 * M_PI);
      str.rpm    = (uint32_t)rpm;
      frame.id   = (CAN_VESC_FLEFT_SET_RPM_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
      frame.size = CAN_VESC_FLEFT_SET_RPM_LENGTH;
      can_vesc_fleft_set_rpm_pack(frame.data, &str, frame.size);
      break;
    }
    case integration_level_t::POSITION: {
      can_vesc_fleft_set_pos_t str;
      double pos = joint_pos[i] * 1000.0; // the value of position is in mDeg | so we need to multiply it by 1000
      str.position = (uint32_t)pos;
      frame.id     = (CAN_VESC_FLEFT_SET_POS_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
      frame.size   = CAN_VESC_FLEFT_SET_POS_LENGTH;
      can_vesc_fleft_set_pos_pack(frame.data, &str, frame.size);
      break;
    }
    case integration_level_t::EFFORT: {
      can_vesc_fleft_set_current_t str;
      double current = joint_effort[i] / current_to_torque * 1000.0;
      str.current    = (uint32_t)current;
      frame.id   = (CAN_VESC_FLEFT_SET_CURRENT_FRAME_ID & 0xffffff00) | ((uint32_t)i + VESC_ID_BASE_START);
      frame.size = CAN_VESC_FLEFT_SET_CURRENT_LENGTH;
      can_vesc_fleft_set_current_pack(frame.data, &str, frame.size);
      break;
    }
    default:
      RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Unknown integration level %d",
                   static_cast<int>(joint.integration_level));
      return hardware_interface::return_type::ERROR;
    }
    // send the frame to the can bus
    can_driver->send(frame);
  }


  for(const auto &[name, descr] : sensor_state_interfaces_) {
    RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Sensor: %s  Desc %s", name.c_str(),
                descr.interface_name.c_str());
  }

  return hardware_interface::return_type::OK;
}

void MCVescHardware::can_callback_status_1(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  joint_t *joint = static_cast<joint_t *>(args);

  can_vesc_fleft_status_1_t status;
  if(can_vesc_fleft_status_1_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack status 1");
    return;
  }
  joint->current    = (double)status.current;
  joint->duty_cycle = (double)status.duty;
  joint->erpm       = (double)status.erpm;
}

void MCVescHardware::can_callback_status_2(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  joint_t *joint = static_cast<joint_t *>(args);
  can_vesc_fleft_status_2_t status;
  if(can_vesc_fleft_status_2_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack status 2");
    return;
  }
  joint->amd_hours         = (double)status.amp_hours * 1000;
  joint->amd_hours_charged = (double)status.amp_hours_chg * 1000;
}

void MCVescHardware::can_callback_status_3(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  joint_t *joint = static_cast<joint_t *>(args);
  can_vesc_fleft_status_3_t status;
  if(can_vesc_fleft_status_3_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack status 3");
    return;
  }
  joint->watt_hours         = (double)status.wat_hours * 1000;
  joint->watt_hours_charged = (double)status.wat_hours_chg * 1000;
}

void MCVescHardware::can_callback_status_4(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  joint_t *joint = static_cast<joint_t *>(args);
  can_vesc_fleft_status_4_t status;
  if(can_vesc_fleft_status_4_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack status 4");
    return;
  }
  joint->current_in         = (double)status.current_in;
  joint->pid_pos            = (double)status.pid_pos;
  joint->temperature_mosfet = (double)status.temp_mosfet;
  joint->temperature_motor  = (double)status.temp_motor;
}

void MCVescHardware::can_callback_status_5(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  joint_t *joint = static_cast<joint_t *>(args);
  can_vesc_fleft_status_5_t status;
  if(can_vesc_fleft_status_5_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack status 5");
    return;
  }
  //  This is weard tachometer shows position the wheel have moved in degrees
  // but for some mysterious resone it shows that value in 180 deg ber 360 deg rotation oven though
  // the cvalue input into the vesc are correct.
  double scale_for_tachometer = 4.0 * M_PI / 360.0; //  2 * 2 * M_PI = 360 deg
  joint->position             = (double)status.tachometer * scale_for_tachometer;
  joint->voltage              = (double)status.volts_in * 0.1;
  // RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "Tachometer: %f  Voltage: %f", joint->tachometer,
  //             joint->voltage);
}

void MCVescHardware::can_callback_status_6(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  joint_t *joint = static_cast<joint_t *>(args);
  can_vesc_fleft_status_6_t status;
  if(can_vesc_fleft_status_6_unpack(&status, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack status 6");
    return;
  }
  joint->adc1 = (double)status.adc1 * 1000;
  joint->adc2 = (double)status.adc2 * 1000;
  joint->adc3 = (double)status.adc3 * 1000;
  joint->ppm  = (double)status.ppm * 1000;
}


#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nomad_hardware::MCVescHardware, hardware_interface::SystemInterface)