import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy, JointState
from std_msgs.msg import Float64MultiArray

class SimpleDriver(Node):
  def __init__(self):
    super().__init__("simple_driver")

    self.sub_joy = self.create_subscription(
        Joy,
        '/joy',
        self.joy_callback,
        10
      )

    self.publisher = self.create_publisher(
      JointState,
      '/joint_command',
      10
    )

  def joy_callback(self, msg: Joy):
    # Joint mapping based on the provided joint list:
    # - right_side_suspension_joint (index 0)
    # - left_side_suspension_joint (index 1)
    # - wheel_rot_right_front_joint (index 2)
    # - wheel_rot_right_rear_joint (index 3)
    # - wheel_rot_left_front_joint (index 4)
    # - wheel_rot_left_rear_joint (index 5)
    # - wheel_right_front_joint (index 6)
    # - wheel_right_rear_joint (index 7)
    # - wheel_left_front_joint (index 8)
    # - wheel_left_rear_joint (index 9)

    joint_commands = [0.0] * 10

    x = msg.axes[0] * 1000
    y = msg.axes[1] * 1000

    if len(msg.axes) >= 2:
      # For suspension joints - use axes[0] for right, axes[1] for left
      joint_commands[0] = 0  # right_side_suspension_joint
      joint_commands[1] = 0  # left_side_suspension_joint

      # For wheel rotation joints (front and rear)
      # Use axes[0] for X scale, axes[1] for Y scale to control rotation
      joint_commands[2] = 0  # wheel_rot_right_front_joint
      joint_commands[3] = 0  # wheel_rot_right_rear_joint
      joint_commands[4] = 0  # wheel_rot_left_front_joint
      joint_commands[5] = 0  # wheel_rot_left_rear_joint

      # For wheel position joints (front and rear)
      # Use axes[1] for Y scale to control position/translation
      joint_commands[6] = 0  # wheel_right_front_joint
      joint_commands[7] = x  # wheel_right_rear_joint
      joint_commands[8] = 0  # left_side_suspension_joint
      joint_commands[9] = 0  # wheel_left_rear_joint

    # Create and publish joint command message
    command_msg = JointState()
    command_msg.header.frame_id="nomad_r"
    command_msg.header.stamp = self.get_clock().now().to_msg()
    command_msg.name = [
        "right_side_suspension_joint",
        "left_side_suspension_joint",
        "wheel_rot_right_front_joint",
        "wheel_rot_right_rear_joint",
        "wheel_rot_left_front_joint",
        "wheel_rot_left_rear_joint",
        "wheel_right_front_joint",
        "wheel_right_rear_joint",
        "wheel_left_front_joint",
        "wheel_left_rear_joint"
    ]
    command_msg.velocity = joint_commands
    command_msg.effort  = [0] * 10
    command_msg.position = [0] * 10

    self.publisher.publish(command_msg)

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
