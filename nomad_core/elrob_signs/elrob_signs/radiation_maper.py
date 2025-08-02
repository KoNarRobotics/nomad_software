import rclpy
from rclpy.node import Node
from nav_msgs.msg import OccupancyGrid
from geometry_msgs.msg import PoseStamped
from nav2_msgs.action import NavigateToPose
from rclpy.action import ActionClient
import numpy as np
from nav_msgs.msg import Odometry

import tf2_ros
import tf2_geometry_msgs
from tf_transformations import euler_from_quaternion
from sensor_msgs.msg import Image
from sensor_msgs.msg import LaserScan
from sensor_msgs.msg import CameraInfo
from geometry_msgs.msg import Point
from std_msgs.msg import Header
from geometry_msgs.msg import PointStamped
from geiger_msg.msg import Radiation

class RadiationMapper(Node):
  def __init__(self):
    super().__init__('RadiationMapper')
    self.get_logger().info("RadiationMapper Started!")
    self.tf_buffer = tf2_ros.Buffer()
    self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)

    self.radiation_sub = self.create_subscription(
      Radiation, '/geiger_state_broadcaster/radiation', self.radiation_callback, 10)
    
    self.map = None
    self.radiation_map = None
    self.map_sub = self.create_subscription(
      OccupancyGrid, '/map', self.map_callback, 10)
    
    self.timer = self.create_timer(1.0, self.publish_radiation_map)
  
  def publish_radiation_map(self):
    if self.map is None or self.radiation_map is None:
      self.get_logger().warn("Map or radiation map not initialized yet.")
      return
    
    # Create a new OccupancyGrid message for the radiation map
    radiation_grid = OccupancyGrid()
    radiation_grid.header.stamp = self.get_clock().now().to_msg()
    radiation_grid.header.frame_id = 'map'
    self.get_logger().info(f"Radiation map size: {self.radiation_map}")
    radiation_grid.info = self.radiation_map.info
    radiation_grid.data = self.radiation_map.data.tolist()  # Convert numpy array to list

    # Publish the radiation map
    self.occ_grid_pub.publish(radiation_grid)
    self.get_logger().info("Radiation map published.")
    
  def map_callback(self, msg: OccupancyGrid):
    self.map = msg
    width = msg.info.width
    height = msg.info.height
    resolution = msg.info.resolution
    if self.radiation_map is None:
      self.radiation_map = self.map
      self.radiation_map.data = np.zeros((int(height*resolution) * int(width*resolution)), dtype=np.int8)
      self.get_logger().info(f"Radiation map initialized with size: w:{width} h:{height}" )
    # else:
      # old_map = self.radiation_map
      # old_height = old_map.info.height
      # old_width  = old_map.info.width
      # new_map = np.zeros(int(old_height*resolution*old_width*resolution), dtype=np.int8)

      # new_map[:min_height, :min_width] = old_map.data[:min_height, :min_width]

      # self.radiation_map.data = new_map
      # self.get_logger().info(f"Radiation map resized to: {self.radiation_map.shape}, old values copied")

  def radiation_callback(self, msg:Radiation):
    # Get the current robot pose from nav2 odometry
    if self.map is None:
      return
    
    try:
      odom_msg = self.tf_buffer.lookup_transform('map', 'base_link', rclpy.time.Time())
      translation = odom_msg.transform.translation
      x = translation.x
      y = translation.y
      xx = int((x - self.radiation_map.info.origin.position.x) / self.radiation_map.info.resolution)
      yy = int((y - self.radiation_map.info.origin.position.y) / self.radiation_map.info.resolution)  
      val = msg.siwert * 1000000 # Convert from Sieverts to microSieverts
      index = yy * xx + xx

      # self.radiation_map.data[yy * xx + xx] = int(val)
      self.get_logger().info(f"Robot pose in map frame: x={x}, y={y} size={len(self.radiation_map.data)} ind:{index}")

    except Exception as e:
      self.get_logger().warn(f"Could not get robot pose: {e}")



def main(args=None):
    rclpy.init(args=args)
    explorer_node = RadiationMapper()
    try:
        explorer_node.get_logger().info("Starting SignDetector...")
        rclpy.spin(explorer_node)
    except KeyboardInterrupt:
        explorer_node.get_logger().info("SignDetecton stopped by user")
    finally:
        explorer_node.destroy_node()
        rclpy.shutdown()
