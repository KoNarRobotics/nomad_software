import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan
from rclpy.qos import QoSProfile, QoSReliabilityPolicy

class ScanRelay(Node):
  def __init__(self):
    super().__init__('scan_relay')

    qos_in = QoSProfile(depth=10, reliability=QoSReliabilityPolicy.BEST_EFFORT)
    qos_out = QoSProfile(depth=10, reliability=QoSReliabilityPolicy.RELIABLE)

    self.pub = self.create_publisher(LaserScan, '/scan', qos_out)
    self.sub = self.create_subscription(LaserScan, '/scan_relay', self.callback, qos_in)

  def callback(self, msg):
    self.pub.publish(msg)

def main():
  rclpy.init()
  node = ScanRelay()
  rclpy.spin(node)
  node.destroy_node()
  rclpy.shutdown()