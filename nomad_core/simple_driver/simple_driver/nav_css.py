import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy, NavSatFix
from geometry_msgs.msg import TwistStamped   
from nav2_msgs.action import NavigateToPose
from rclpy.action import ActionClient
from nav_msgs.msg import Odometry   




from pprint import pprint

class SimpleDriver(Node):
  def __init__(self):
    super().__init__("simple_driver")


    self.declare_parameter("kaganiec_predkosci", 0.1)
    self.nav_to_pose_client = ActionClient(self, NavigateToPose, 'navigate_to_pose')
    self.canceling_goal  =False

    self.file = open("/home/lemonx/it/nomad_software/nomad_core/simple_driver/simple_driver/gps_log_3.csv","w")

    self.sub_joint_states = self.create_subscription(
        NavSatFix ,
        '/ublox_gps_node/fix',
        self.odom_callback,
        10
      )
    self.count = 0

  def destroy_node(self):
    self.file.close()
    return super().destroy_node()

  def odom_callback(self, msg: NavSatFix):
    lat = msg.latitude + 0.00005
    lon = msg.longitude + 0.00005
    msg_time = self.get_clock().now().to_msg()
    msg_send_time = msg.header.stamp

    s = f"{self.count}, {lat}, {lon}\n"
    self.count += 1
    self.file.write(s)
    self.file.flush()
    self.get_logger().info(s + f" msg_time: {msg_time.sec}.{msg_time.nanosec} msg_send_time: {msg_send_time.sec}.{msg_send_time.nanosec} diff: {(msg_time.sec - msg_send_time.sec)*1e9 + (msg_time.nanosec - msg_send_time.nanosec)} ns")






def main(args=None):
  rclpy.init(args=args)
  sdnode = SimpleDriver()
  try:
    rclpy.spin(sdnode)
  except KeyboardInterrupt:
    pass
  finally:
    sdnode.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
  main()