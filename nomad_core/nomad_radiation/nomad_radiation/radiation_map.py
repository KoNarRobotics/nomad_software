import rclpy
from rclpy.node import Node
from nav_msgs.msg import OccupancyGrid
from geometry_msgs.msg import Pose
from std_msgs.msg import Header
from geiger_msg.msg import Radiation 
import tf2_ros
import numpy as np
from tf2_ros import TransformException
import tf_transformations
from geometry_msgs.msg import TransformStamped

class RadiationMapper(Node):
    def __init__(self):
        super().__init__('radiation_mapper')

        self.declare_parameter('resolution', 0.2)
        self.declare_parameter('max_size_x', 200)
        self.declare_parameter('max_size_y', 200)

        self.resolution = self.get_parameter('resolution').get_parameter_value().double_value
        self.size_x = self.get_parameter('max_size_x').get_parameter_value().integer_value
        self.size_y = self.get_parameter('max_size_y').get_parameter_value().integer_value

        self.origin_x = - (self.size_x * self.resolution) / 2.0
        self.origin_y = - (self.size_y * self.resolution) / 2.0

        self.heatmap = np.zeros((self.size_x, self.size_y))

        self.tf_buffer = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)

        self.subscription = self.create_subscription(
            Radiation,
            '/radiation',
            self.callback,
            10)

        self.publisher = self.create_publisher(OccupancyGrid, '/radiation_heatmap', 10)

        self.timer = self.create_timer(1.0, self.publish_map)

    def callback(self, msg):
        try:
            transform = self.tf_buffer.lookup_transform(
                'map',
                msg.header.frame_id,
                rclpy.time.Time(),  # <<< this requests the latest available transform
                timeout=rclpy.duration.Duration(seconds=0.5)
            )
            x, y = transform.transform.translation.x, transform.transform.translation.y

            # Convert to grid
            gx = int((x - self.origin_x) / self.resolution)
            gy = int((y - self.origin_y) / self.resolution)

            if 0 <= gx < self.size_x and 0 <= gy < self.size_y:
                val = msg.siwert
                if np.isnan(val):
                  self.get_logger().error("Received NaN value for radiation")
                self.heatmap[gx, gy] = min(100.0, val * 100.0)  # Scale to 0–100

        except TransformException as ex:
            self.get_logger().warn(f"TF transform failed: {ex}")

    def publish_map(self):
        msg = OccupancyGrid()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'map'

        msg.info.resolution = self.resolution
        msg.info.width = self.size_x
        msg.info.height = self.size_y

        msg.info.origin.position.x = self.origin_x
        msg.info.origin.position.y = self.origin_y
        msg.info.origin.position.z = 0.0

        msg.info.origin.orientation.w = 1.0

        flat = self.heatmap.T.flatten()
        msg.data = [int(min(100, max(0, v))) for v in flat]

        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = RadiationMapper()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
