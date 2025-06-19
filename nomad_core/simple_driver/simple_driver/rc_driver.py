import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from geometry_msgs.msg import TwistStamped   
# geometry_msgs/msg/TwistStamped


from pprint import pprint

class SimpleDriver(Node):
  def __init__(self):
    super().__init__("simple_driver")


    self.declare_parameter("kaganiec_predkosci", 0.1)


    self.sub_joint_states = self.create_subscription(
        Joy,
        '/joy',
        self.joy_callback,
        10
      )
    
    self.publisher = self.create_publisher(
      TwistStamped,
      '/cmd_vel',
      10
    )

  def joy_callback(self, msg: Joy):
    kaganiec = self.get_parameter("kaganiec_predkosci").value
    if msg.buttons[8] == 1:
      kaganiec = 1
      self.get_logger().warning("TURBO Activated")
    

    max_velocity  = 13.0 * kaganiec # m/s
    max_angular_velocity = kaganiec * 12.56 


    x = msg.axes[0] * max_velocity
    ang_z = msg.axes[2] * max_angular_velocity 
    


    msg_cmd = TwistStamped()
    msg_cmd.header.frame_id = "base_link"
    msg_cmd.header.stamp = self.get_clock().now().to_msg()
    msg_cmd.twist.linear.x = x
    msg_cmd.twist.linear.y = 0.0
    msg_cmd.twist.linear.z = 0.0
    msg_cmd.twist.angular.x = 0.0
    msg_cmd.twist.angular.y = 0.0
    msg_cmd.twist.angular.z = ang_z
    # self.get_logger().info(str(msg_cmd))
    self.publisher.publish(msg_cmd)



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