import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy
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


    self.sub_joint_states = self.create_subscription(
        Odometry ,
        '/odom_in',
        self.odom_callback,
        10
      )
    
    self.publisher = self.create_publisher(
      Odometry ,
      '/odom_out',
      10
    )

  def odom_callback(self, msg: Odometry):
    msg_cmd = Odometry()
    msg_cmd.header.frame_id = "odom"
    msg_cmd.child_frame_id = "base_link"
    msg_cmd.header.stamp = msg.header.stamp
    msg_cmd.pose = msg.pose
    msg_cmd.twist = msg.twist
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