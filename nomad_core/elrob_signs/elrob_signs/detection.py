import rclpy
from rclpy.node import Node
from nav_msgs.msg import OccupancyGrid
from geometry_msgs.msg import PoseStamped
from nav2_msgs.action import NavigateToPose
from rclpy.action import ActionClient
import numpy as np
from nav_msgs.msg import Odometry
from cv_bridge import CvBridge
import cv2

import tf2_ros
import tf2_geometry_msgs
from tf_transformations import euler_from_quaternion
from sensor_msgs.msg import Image
from sensor_msgs.msg import LaserScan
from sensor_msgs.msg import CameraInfo
from geometry_msgs.msg import Point
from std_msgs.msg import Header
from geometry_msgs.msg import PointStamped


def detect_orange_plates(image, show_result=True, min_area=1000):
  """
  Detect orange plates in a BGR image using HSV color thresholding.
  
  Args:
      image (np.ndarray): Input BGR image (cv2 format).
      show_result (bool): If True, display the detection image in a window.
      min_area (int): Minimum area of contour to be considered a valid detection.
  
  Returns:
      result_image (np.ndarray): Image with detections drawn.
      centers (list of (x, y)): List of center coordinates of detected orange regions.
  """

  # Define orange HSV range
  lower_orange = np.array([1, 80, 80])
  upper_orange = np.array([25, 255, 255])

  # Convert to HSV and create mask
  hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
  mask = cv2.inRange(hsv, lower_orange, upper_orange)

  # Find contours
  contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

  centers = []
  result_image = image.copy()

  for cnt in contours:
      area = cv2.contourArea(cnt)
      if area > min_area:
          M = cv2.moments(cnt)
          if M["m00"] != 0:
              cx = int(M["m10"] / M["m00"])
              cy = int(M["m01"] / M["m00"])
              centers.append((cx, cy))
              # Draw detection
              cv2.drawContours(result_image, [cnt], -1, (0, 255, 0), 2)
              cv2.circle(result_image, (cx, cy), 5, (255, 0, 0), -1)
              cv2.putText(result_image, f"({cx},{cy})", (cx+10, cy-10),
                          cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)

  if show_result:
      cv2.imshow("Orange Plate Detection", result_image)
      cv2.waitKey(0)
      cv2.destroyAllWindows()

  return result_image, centers



class SignDetector(Node):
  def __init__(self):
    super().__init__('SignDetector')
    self.get_logger().info("SignDetector Started")
    self.tf_buffer = tf2_ros.Buffer()
    self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)
    self.bridge = CvBridge()

    self.image_pub = self.create_publisher(Image, '/detected_sign/image', 10)
    self.position_pub = self.create_publisher(PointStamped, '/detected_sign/position', 10)

    self.camera_info_sub_right = self.create_subscription(
      CameraInfo, '/camera_info', self.camera_info_callback, 10)

    self.image_sub = self.create_subscription(
      Image, '/image_raw', self.image_callback, 10)

    self.scan_subscription = self.create_subscription(
      LaserScan, '/scan', self.laser_scan_callback, 10)
 
    self.laser_scan:LaserScan = LaserScan() 
    self.camera_info:CameraInfo = CameraInfo()

  def sign_detect(self,msg:Image):
    frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

    # Convert to HSV color space
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    # Define orange color range
    lower_orange = np.array([10, 100, 100])
    upper_orange = np.array([25, 255, 255])
    # Threshold the HSV image to get only orange colors
    mask = cv2.inRange(hsv, lower_orange, upper_orange)

    # Find contours
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    for cnt in contours:
      area = cv2.contourArea(cnt)
      if area > 1000:  # Filter small areas
        M = cv2.moments(cnt)
        if M["m00"] != 0:
          cx = int(M["m10"] / M["m00"])
          cy = int(M["m01"] / M["m00"])
          # Draw contour and center
          cv2.drawContours(frame, [cnt], -1, (0, 255, 0), 2)
          cv2.circle(frame, (cx, cy), 5, (255, 0, 0), -1)
          cv2.putText(frame, f"({cx},{cy})", (cx+10, cy-10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,255), 2)
          self.get_logger().info(f"Orange plate detected at: x={cx}, y={cy}")


  def camera_info_callback(self,msg:CameraInfo):
    self.camera_info = msg

  def laser_scan_callback(self,msg:LaserScan):
    self.laser_scan = msg

  def detect_sign(self,msg:Image):
    frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
    image, centers = detect_orange_plates(frame, show_result=False, min_area=1000)
    if len(centers) == 0:
      self.get_logger().warn("No orange plates detected")
      return None, None
    x =0
    y = 0
    for center in centers:
      x += center[0]
      y += center[1]
    x = int(x / len(centers))
    y = int(y / len(centers))
    self.get_logger().info(f"Detected orange plate at Pixels: x={x}, y={y}")
    return x,y

  def image_to_map_coords(self, u, v, camera_info, camera_frame):
    """
    Transforms image pixel coordinates (u, v) from a camera to map coordinates.

    Args:
      u (int): Pixel x-coordinate in the image.
      v (int): Pixel y-coordinate in the image.
      camera_info (sensor_msgs.msg.CameraInfo): Camera calibration info.
      camera_frame (str): The frame id of the camera.

    Returns:
      (float, float): (x, y) coordinates in the map frame, or None if transform fails.
    """
    # Camera intrinsics
    fx = camera_info.k[0]
    fy = camera_info.k[4]
    cx = camera_info.k[2]
    cy = camera_info.k[5]

    # Assume a fixed depth (e.g., 1 meter) for projection
    Z = 1.0
    X = (u - cx) * Z / fx
    Y = (v - cy) * Z / fy

    # Create a PoseStamped in the camera frame
    pose_cam = PoseStamped()
    pose_cam.header.stamp = self.get_clock().now().to_msg()
    pose_cam.header.frame_id = camera_frame
    pose_cam.pose.position.x = X
    pose_cam.pose.position.y = Y
    pose_cam.pose.position.z = Z
    pose_cam.pose.orientation.w = 1.0

    # Transform to base_link
    try:
      pose_base = self.tf_buffer.transform(pose_cam, 'base_link', timeout=rclpy.duration.Duration(seconds=0.5))
    except Exception as e:
      self.get_logger().warn(f"TF transform to base_link failed: {e}")
      return None,None

    # Project to closest point on the wall using lidar/scan data (if available)
    
    
    # For now, just use the transformed point as is
    # Project the point from the camera as a straight line to a wall using scan/cloud data

    # Get the origin of the camera in base_link frame
    camera_origin = np.array([0.0, 0.0, 0.0])  # Camera frame origin
    point_in_base = np.array([
      pose_base.pose.position.x,
      pose_base.pose.position.y,
      pose_base.pose.position.z
    ])

    # Direction vector from camera origin to projected point in base_link
    direction = point_in_base - camera_origin
    direction = direction / np.linalg.norm(direction)

    # Use latest lidar/scan data to find intersection with wall
    # For this example, assume self.latest_scan is a LaserScan message
    # and self.latest_scan_pose is the pose of the scan in base_link
    # (You need to subscribe to /scan and store the latest message in self.latest_scan)

    intersection_point = None
    if hasattr(self, 'latest_scan') and self.laser_scan is not None:
      scan = self.laser_scan
      angle_min = scan.angle_min
      angle_increment = scan.angle_increment
      ranges = np.array(scan.ranges)

      # For each scan beam, compute its direction in base_link
      for i, r in enumerate(ranges):
        if np.isinf(r) or np.isnan(r):
          continue
        angle = angle_min + i * angle_increment
        # 2D scan, so z=0
        scan_point = np.array([r * np.cos(angle), r * np.sin(angle), 0.0])
        # Compute if the scan_point lies close to the ray from camera
        # Find the closest point on the ray to the scan_point
        t = np.dot(scan_point - camera_origin, direction)
        closest_point = camera_origin + t * direction
        dist = np.linalg.norm(scan_point - closest_point)
        if dist < 0.2:  # Threshold for intersection (meters)
          intersection_point = scan_point
          break

    if intersection_point is not None:
      # Update pose_base to intersection point
      pose_base.pose.position.x = intersection_point[0]
      pose_base.pose.position.y = intersection_point[1]
      pose_base.pose.position.z = intersection_point[2]
    # else: keep pose_base as is (no intersection found)
    # You can add logic here to project onto the wall using scan data

    # Transform to map frame
    try:
      pose_map = self.tf_buffer.transform(pose_base, 'map', timeout=rclpy.duration.Duration(seconds=0.5))
    except Exception as e:
      self.get_logger().warn(f"TF transform to map failed: {e}")
      return (None,None)

    x_map = pose_map.pose.position.x
    y_map = pose_map.pose.position.y
    return (x_map, y_map)

  def image_callback(self,msg:Image):
    x,y = self.detect_sign(msg)
    if x is None or y is None:
      return
    x_map, y_map = self.image_to_map_coords(x,y,self.camera_info,msg)
    if x_map is None or y_map is None:
      return
    pos = PointStamped()
    pos.header.frame_id = 'map'
    pos.header.stamp = msg.header.stamp
    pos.point.x = x_map
    pos.point.y = y_map
    pos.point.z = 0.0
    self.image_pub.publish(msg)
    self.position_pub.publish(pos)


def main(args=None):
    rclpy.init(args=args)
    explorer_node = SignDetector()
    try:
        explorer_node.get_logger().info("Starting SignDetector...")
        rclpy.spin(explorer_node)
    except KeyboardInterrupt:
        explorer_node.get_logger().info("SignDetecton stopped by user")
    finally:
        explorer_node.destroy_node()
        rclpy.shutdown()

# if __name__ == "__main__":
#   images = [
#     '/home/lemonx/Pictures/Screenshot_20250702_013859.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013852.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013847.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013837.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013833.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013829.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013823.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013817.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013408.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013359.png',
#     '/home/lemonx/Pictures/Screenshot_20250702_013340.png'
#   ]
#   for i in images:
#     image = cv2.imread(i)
#     x,y = detect_orange_plates(image,True, min_area=1000)