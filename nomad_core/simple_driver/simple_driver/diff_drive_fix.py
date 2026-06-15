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

    self.turn_val = 0

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

    joint_commands_pos = [0.0] * 10
    joint_commands_vel = [0.0] * 10

    wheels = msg.axes[0] * 20


    x2 = msg.axes[2] * 1

    self.turn_val += x2 * 0.04

    # x3 = msg.axes[2] * 1000
    # x4 = msg.axes[3] * 1000
    # x5 = msg.axes[4] * 1000
    # x6 = msg.axes[5] * 1000
    # x7 = msg.axes[6] * 1000
    # x8 = msg.axes[7] * 1000
    self.get_logger().info(f"x2:{x2} wheel:{wheels}")

    if len(msg.axes) >= 2:
      # For suspension joints - use axes[0] for right, axes[1] for left
      joint_commands_pos[0] = 0  # right_side_suspension_joint
      joint_commands_pos[1] = 0  # left_side_suspension_joint

      # For wheel rotation joints (front and rear)
      # Use axes[0] for X scale, axes[1] for Y scale to control rotation
      joint_commands_pos[2] = self.turn_val  # wheel_rot_right_front_joint
      joint_commands_pos[3] = self.turn_val  # wheel_rot_right_rear_joint
      joint_commands_pos[4] = self.turn_val  # wheel_rot_left_front_joint
      joint_commands_pos[5] = self.turn_val  # wheel_rot_left_rear_joint

      # For wheel position joints (front and rear)
      # Use axes[1] for Y scale to control position/translation
      joint_commands_vel[6] = wheels  # wheel_right_front_joint
      joint_commands_vel[7] = wheels # wheel_right_rear_joint
      joint_commands_vel[8] = -wheels # left_side_suspension_joint
      joint_commands_vel[9] = -wheels # wheel_left_rear_joint

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
    command_msg.velocity = joint_commands_vel
    command_msg.effort  = [0] * 10
    command_msg.position = joint_commands_pos

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
