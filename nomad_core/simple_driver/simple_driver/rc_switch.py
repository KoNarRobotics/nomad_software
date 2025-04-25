import rclpy
from rclpy.duration import Duration
from rclpy.node import Node
from sensor_msgs.msg import Joy
from enum import Enum

import can
import cantools
import time
from pprint import pprint

  
class PassRC(Enum):
  NOMAD = 1
  KONAR = 2

class RCSwitch(Node):
  def __init__(self):
    super().__init__("rc_switch")

    self.sub_joint_states = self.create_subscription(
        Joy,
        '/joy',
        self.joy_callback,
        10
      )
    
    nomad_joy_pub = self.create_publisher(Joy, "/nomad/joy", 10)
    konar_joy_pub = self.create_publisher(Joy, "/sdrac/joy", 10)

    self.buttons_previous = [0,0,0,0,0,0,0,0]
    self.joy_pass_topics={
      PassRC.KONAR: konar_joy_pub,
      PassRC.NOMAD: nomad_joy_pub
    }  
    self.pass_messages_to = PassRC.NOMAD

  def check_if_button_pressed(self, buttons, index):
    pressed = False
    if buttons[index] != self.buttons_previous[index] and buttons[index] == 1:
      pressed = True
    self.buttons_previous[index] = buttons[index]
    return pressed
  

  def joy_callback(self, msg: Joy):
    # when buton 6 is pressed we switch next controler
    if self.check_if_button_pressed(msg.buttons, 6):
      self.get_logger().info("Switching to konar")
      self.pass_messages_to = PassRC.KONAR
    
    # when buton 7 is pressed we switch next controler
    if self.check_if_button_pressed(msg.buttons, 7):
      self.get_logger().info("Switching to nomad")
      self.pass_messages_to = PassRC.NOMAD
    ms = Joy()
    ms.header.stamp = self.get_clock().now().to_msg()
    ms.header.frame_id = msg.header.frame_id
    ms.axes = msg.axes
    ms.buttons = msg.buttons
    self.joy_pass_topics[self.pass_messages_to].publish(msg)

def main(args=None):
  rclpy.init(args=args)
  sdnode = RCSwitch()
  try:
    rclpy.spin(sdnode)
  except KeyboardInterrupt:
    pass
  finally:
    sdnode.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
  main()