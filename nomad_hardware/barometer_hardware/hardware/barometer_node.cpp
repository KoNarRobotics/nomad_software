#include <rclcpp/rclcpp.hpp>


class BarometerNode : public rclcpp::Node {
public:
  BarometerNode() : Node("barometer_node") {

    timer_ = this->create_wall_timer(std::chrono::microseconds(500),
                                     std::bind(&BarometerNode::read_and_print_barometer, this));
  }

private:
  void read_and_print_barometer() {
    // Simulate reading from the barometer
    float pressure    = 1013.25; // Example pressure value in hPa
    float temperature = 25.0;    // Example temperature value in Celsius

    RCLCPP_INFO(this->get_logger(), "Pressure: %.2f hPa, Temperature: %.2f °C", pressure, temperature);
  }

  rclcpp::TimerBase::SharedPtr timer_;
};


int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<BarometerNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}