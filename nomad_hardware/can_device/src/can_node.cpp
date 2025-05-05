#include "rclcpp/rclcpp.hpp"
#include "rclcpp/logging.hpp"
#include "can_device/can_device.hpp"
#include "can_messages.h"

using namespace nomad_hardware;

class CanNode : public rclcpp::Node {
public:
  CanNode() : Node("can_node") {
    RCLCPP_INFO(this->get_logger(), "CAN Node has been started.");
    can_driver = CanDriver::Make("can0", true, 1000, 10).valueOrDie();

    can_driver->open_can();
  }

  ~CanNode() {
    can_driver->close_can();
  }

  std::shared_ptr<CanDriver> can_driver;


  float barometer_pressure;
  float barometer_temperature;
  int barometer_status;

  void can_callback_status(CanDriver &can, const CanFrame &frame, void *args) {
    (void)can;
    (void)args;

    can_barometer_status_t status;
    if(can_barometer_status_unpack(&status, frame.data, frame.size)) {
      RCLCPP_ERROR(rclcpp::get_logger("CanNode"), "Failed to unpack barometer status");
      return;
    }
    barometer_status = status.status;
  }

  void can_callback_data(CanDriver &can, const CanFrame &frame, void *args) {
    (void)can;
    (void)args;

    can_barometer_data_t data;
    if(can_barometer_data_unpack(&data, frame.data, frame.size)) {
      RCLCPP_ERROR(rclcpp::get_logger("CanNode"), "Failed to unpack barometer data");
      return;
    }
    barometer_temperature = data.temperature;
    barometer_pressure    = data.pressure;
  }
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CanNode>());
  rclcpp::shutdown();
  return 0;
}