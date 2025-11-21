import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
from geometry_msgs.msg import TwistStamped   
from nav2_msgs.action import NavigateToPose
from rclpy.action import ActionClient
# geometry_msgs/msg/TwistStamped


from pprint import pprint

class SimpleDriver(Node):
  def __init__(self):
    super().__init__("simple_driver")


    self.declare_parameter("kaganiec_predkosci", 0.1)
    self.nav_to_pose_client = ActionClient(self, NavigateToPose, 'navigate_to_pose')
    self.canceling_goal  =False


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

  def goal_canceled_callback(self):
    self.canceling_goal = False
    pass

  def joy_callback(self, msg: Joy):
    kaganiec = self.get_parameter("kaganiec_predkosci").get_parameter_value().double_value
    if msg.buttons[8] == 1:
      kaganiec = 1
      self.get_logger().warning("TURBO Activated")
    

    max_velocity  = 3.5 * kaganiec # m/s
    max_angular_velocity = kaganiec * 1.428571429


    x = msg.axes[0] * max_velocity
    ang_z = -msg.axes[1] * max_angular_velocity 
    

    if msg.buttons[5]==1 and not self.canceling_goal:
      self.canceling_goal = True
      self.get_logger().info("Gol Canceling by Joystick")
      self.nav_to_pose_client._cancel_goal_async(self.goal_canceled_callback)

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