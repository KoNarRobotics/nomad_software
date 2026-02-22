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

#include "gps_hardware/gps_hardware.hpp"

#include "sensor_msgs/msg/detail/nav_sat_status__struct.hpp"

using namespace nomad_hardware;

namespace {
constexpr const char *nameHardwareInterface = "MCGpsHardware";
}


#define NOMAD_RC_RETURN_ON_ERROR_MSG(result, msg, name)   \
  do {                                                    \
    if(result) {                                          \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), msg); \
      return hardware_interface::CallbackReturn::ERROR;   \
    }                                                     \
  } while(false);

#define NOMAD_RC_RETURN_ON_ERROR_NAM(result, name)                           \
  do {                                                                       \
    Status _x = result.status();                                             \
    if(!_x.ok()) {                                                           \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), _x.to_string().c_str()); \
      return hardware_interface::CallbackReturn::ERROR;                      \
    }                                                                        \
  } while(false);


hardware_interface::CallbackReturn MCGpsHardware::on_init(const hardware_interface::HardwareInfo &info) {
  if(hardware_interface::SensorInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(rclcpp::get_logger(nameHardwareInterface), "MCGpsHardware creating...");

  auto mayby_can_interface = info_.hardware_parameters.find("can_interface");
  NOMAD_RC_RETURN_ON_ERROR_MSG(mayby_can_interface != info_.hardware_parameters.end(), nameHardwareInterface,
                               "No CAN interface specified");

  auto can_interface_name = mayby_can_interface->second;
  NOMAD_RC_RETURN_ON_ERROR_MSG(!can_interface_name.empty(), nameHardwareInterface, "CAN interface is empty");

  auto mayby_can_driver = CanDriver::Make(can_interface_name);
  if(!mayby_can_driver.ok()) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger(nameHardwareInterface), "Failed to create CAN driver");
    return hardware_interface::CallbackReturn::ERROR;
  }
  can_driver = mayby_can_driver.valueOrDie();


  auto sensor_it =
  std::find_if(info_.sensors.begin(), info_.sensors.end(),
               [](const hardware_interface::ComponentInfo &sensor) { return sensor.name == "gps"; });
  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it != info_.sensors.end(), nameHardwareInterface, "gps not found in hardware info");

  NOMAD_RC_RETURN_ON_ERROR_MSG(sensor_it->state_interfaces.size() == 14, nameHardwareInterface,
                               "gps should have 14 state interfaces");
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGpsHardware::on_configure(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GPS_STATUS_FRAME_ID,
                                                        std::bind(&MCGpsHardware::can_callback_status, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GPS_LATITUDE_FRAME_ID,
                                                        std::bind(&MCGpsHardware::can_callback_latitude, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GPS_LONGITUDE_FRAME_ID,
                                                        std::bind(&MCGpsHardware::can_callback_longitude, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GPS_ALTITUDE_FRAME_ID,
                                                        std::bind(&MCGpsHardware::can_callback_altitude, this,
                                                                  std::placeholders::_1, std::placeholders::_2,
                                                                  std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GPS_DATE_FRAME_ID,
                                                        std::bind(&MCGpsHardware::can_callback_date, this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);

  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->add_callback(CAN_GPS_COVARIANCE_FRAME_ID,
                                                        std::bind(&MCGpsHardware::can_callback_covariance,
                                                                  this, std::placeholders::_1,
                                                                  std::placeholders::_2, std::placeholders::_3),
                                                        nullptr),
                               nameHardwareInterface);


  for(const auto &[name, descr] : sensor_state_interfaces_) {
    set_state(name, 0.0);
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGpsHardware::on_activate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "on_activate");
  longitude            = std::numeric_limits<double>::quiet_NaN();
  latitude             = std::numeric_limits<double>::quiet_NaN();
  altitude             = std::numeric_limits<double>::quiet_NaN();
  latitude_covariance  = std::numeric_limits<double>::quiet_NaN();
  longitude_covariance = std::numeric_limits<double>::quiet_NaN();
  altitude_covariance  = std::numeric_limits<double>::quiet_NaN();
  std::time_t now      = std::time(nullptr);
  date_time            = *std::localtime(&now);
  status               = sensor_msgs::msg::NavSatStatus::STATUS_NO_FIX;
  signal_type          = sensor_msgs::msg::NavSatStatus::SERVICE_UNKNOWN;
  NOMAD_RC_RETURN_ON_ERROR_NAM(can_driver->open_can(), nameHardwareInterface);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MCGpsHardware::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  (void)previous_state;
  RCLCPP_DEBUG(rclcpp::get_logger(nameHardwareInterface), "on_deactivate");
  NOMAD_RC_RETURN_ON_ERROR_NAM(CanDriver::close_can(can_driver), nameHardwareInterface);
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type MCGpsHardware::read(const rclcpp::Time &time, const rclcpp::Duration &period) {
  (void)time;
  (void)period;
  CanFrame frame;
  frame.id                = CAN_GPS_STATUS_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_GPS_STATUS_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [Status] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id                = CAN_GPS_LATITUDE_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_GPS_LATITUDE_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [latitude] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id                = CAN_GPS_LONGITUDE_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_GPS_LONGITUDE_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [longitude] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id                = CAN_GPS_ALTITUDE_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_GPS_ALTITUDE_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [altitude] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id                = CAN_GPS_DATE_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_GPS_DATE_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [date] CAN frame");
    return hardware_interface::return_type::ERROR;
  };

  frame.id                = CAN_GPS_COVARIANCE_FRAME_ID;
  frame.size              = 0;
  frame.is_remote_request = true;
  frame.is_extended       = CAN_GPS_COVARIANCE_IS_EXTENDED;
  if(!can_driver->send(frame).ok()) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to send [covariance] CAN frame");
    return hardware_interface::return_type::ERROR;
  };


  if(stat_t == MCGpsHardwareStatus::DISCONNECTED) {
    latitude  = std::numeric_limits<double>::quiet_NaN();
    longitude = std::numeric_limits<double>::quiet_NaN();
    altitude  = std::numeric_limits<double>::quiet_NaN();
  }

  set_state("gps/status", static_cast<double>(status));
  set_state("gps/service", static_cast<double>(signal_type));
  set_state("gps/latitude", latitude);
  set_state("gps/longitude", longitude);
  set_state("gps/altitude", altitude);
  set_state("gps/latitude_covariance", latitude_covariance);
  set_state("gps/longitude_covariance", longitude_covariance);
  set_state("gps/altitude_covariance", altitude_covariance);
  set_state("gps/year", static_cast<double>(date_time.tm_year));
  set_state("gps/month", static_cast<double>(date_time.tm_mon));
  set_state("gps/day", static_cast<double>(date_time.tm_mday));
  set_state("gps/hour", static_cast<double>(date_time.tm_hour));
  set_state("gps/minute", static_cast<double>(date_time.tm_min));
  set_state("gps/second", static_cast<double>(date_time.tm_sec));


  return hardware_interface::return_type::OK;
}

void MCGpsHardware::can_callback_status(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_gps_status_t st;
  if(can_gps_status_unpack(&st, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack GPS status");
    return;
  }

  stat_t = static_cast<MCGpsHardwareStatus>(st.status);

  switch(stat_t) {
  case MCGpsHardwareStatus::OK: status = sensor_msgs::msg::NavSatStatus::STATUS_FIX; break;
  case MCGpsHardwareStatus::DISCONNECTED: status = sensor_msgs::msg::NavSatStatus::STATUS_NO_FIX; break;
  case MCGpsHardwareStatus::ERROR: status = sensor_msgs::msg::NavSatStatus::STATUS_UNKNOWN; break;
  }

  switch(static_cast<MCGpsSignalType>(st.signal)) {
  case MCGpsSignalType::GROUND_BASED_AUGMENTATION:
    signal_type = sensor_msgs::msg::NavSatStatus::SERVICE_GPS;
    break;
  case MCGpsSignalType::SATELLITE_BASED_AUGMENTATION:
    signal_type = sensor_msgs::msg::NavSatStatus::SERVICE_GPS;
    break;
  case MCGpsSignalType::UNABLE_TO_FIX_POSITION:
    signal_type = sensor_msgs::msg::NavSatStatus::SERVICE_UNKNOWN;
    break;
  case MCGpsSignalType::UNAUGMENTED_FIX: signal_type = sensor_msgs::msg::NavSatStatus::SERVICE_GPS; break;
  }
}

void MCGpsHardware::can_callback_latitude(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_gps_latitude_t lat;
  if(can_gps_latitude_unpack(&lat, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack GPS latitude");
    return;
  }
  latitude = lat.latitude;
}

void MCGpsHardware::can_callback_longitude(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_gps_longitude_t lon;
  if(can_gps_longitude_unpack(&lon, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack GPS longitude");
    return;
  }
  longitude = lon.longitude;
}

void MCGpsHardware::can_callback_altitude(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_gps_altitude_t alt;
  if(can_gps_altitude_unpack(&alt, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack GPS altitude");
    return;
  }
  altitude = alt.altitude;
}

void MCGpsHardware::can_callback_date(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_gps_date_t date;
  if(can_gps_date_unpack(&date, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack GPS date");
    return;
  }
  date_time.tm_year = date.year;  // tm_year is years since 1900
  date_time.tm_mon  = date.month; // tm_mon is months since January
  date_time.tm_mday = date.day;
  date_time.tm_hour = date.hour;
  date_time.tm_min  = date.minute;
  date_time.tm_sec  = date.second;
}

void MCGpsHardware::can_callback_covariance(CanDriver &can, const CanFrame &frame, void *args) {
  (void)can;
  (void)args;

  can_gps_covariance_t cov;
  if(can_gps_covariance_unpack(&cov, frame.data, frame.size)) {
    RCLCPP_ERROR(rclcpp::get_logger(nameHardwareInterface), "Failed to unpack GPS covariance");
    return;
  }
  latitude_covariance  = cov.lat;
  longitude_covariance = cov.lon;
  altitude_covariance  = cov.alt;
}


#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nomad_hardware::MCGpsHardware, hardware_interface::SensorInterface)