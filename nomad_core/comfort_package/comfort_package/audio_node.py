import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TwistStamped
from comfort_package.electric_motor_synthesizer import ElectricMotorSynth 


class AudioComforterSubscriber(Node):
  def __init__(self):
    super().__init__('minimal_subscriber')
    self.param_max_velocity = self.declare_parameter('max_velocity', 2.5)
    self.max_velocity = self.param_max_velocity.get_parameter_value().double_value
    self.param_audio_samplera = self.declare_parameter('audio_samplera', 48000)
    self.audio_samplera = self.param_audio_samplera.get_parameter_value().integer_value

    self.subscription = self.create_subscription(
      TwistStamped,
      'cmd_vel',
      self.msg_callback,
      10)
    try:
      self.synth = ElectricMotorSynth(samplerate=self.audio_samplera)
      self.synth.set_level(0.0)
    except Exception as e:
      self.get_logger().error(f"Failed to start audio synth: {e}")
      self.synth = None

  def msg_callback(self, msg: TwistStamped):
    if self.synth is None:
      return
    linear_speed =float(msg.twist.linear.x)
    level = abs(linear_speed) / self.max_velocity
    self.synth.set_level(level) 

def main(args=None):
  rclpy.init(args=args)
  minimal_subscriber = AudioComforterSubscriber()
  rclpy.spin(minimal_subscriber)
  minimal_subscriber.destroy_node()
  rclpy.shutdown()


if __name__ == '__main__':
  main()