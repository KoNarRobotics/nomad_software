from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration
from launch.actions import SetEnvironmentVariable

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource

import os
import subprocess

def is_video_capture_device(device_path):
    """Check if device is a video capture device (camera)"""
    try:
        result = subprocess.run(['v4l2-ctl', '--device', device_path, '--info'], 
                              capture_output=True, text=True, check=True)
        # Look for Video Capture capability
        return 'Video Capture' in result.stdout and 'Video Output' not in result.stdout
    except:
        return False


def scan_for_camera_devices():
  video_devices = []
  
  try:
      # Get device list from v4l2-ctl
      result = subprocess.run(['v4l2-ctl', '--list-devices'], 
                            capture_output=True, text=True, check=True)
      
      lines = result.stdout.split('\n')
      current_device = None
      
      for line in lines:
          line = line.strip()
          if ':' in line and 'usb-' in line:
              # This is a device name line
              current_device = line.split(':')[0].strip()
          elif line.startswith('/dev/video') and current_device:
              device_path = line.strip()
              # Check if this is a capture device (camera)
              if is_video_capture_device(device_path):
                  video_devices.append({
                      'device': device_path,
                      'name': current_device
                  })
  
  except subprocess.CalledProcessError:
      # Fallback to simple directory scan
      for i in range(10):
          device_path = f"/dev/video{i}"
          if os.path.exists(device_path) and is_video_capture_device(device_path):
              video_devices.append({
                  'device': device_path,
                  'name': f'Camera{i}'
              })
  
  return video_devices

# Condif base on package usb_cam
# https://github.com/ros-drivers/usb_cam

def generate_launch_description():
  frame_rate = 15
  width = 1280
  height = 720

  environment_variable = [
    SetEnvironmentVariable(name="FASTDDS_BUILTIN_TRANSPORTS",value="LARGE_DATA")
    ]

  device = ""
  # Your other nodes
  camera_front = Node(
    package="usb_cam",
    executable="usb_cam_node_exe",
    namespace="camera_front",
    parameters=[{
      "image_width": width,
      "image_height": height,
      "framerate": frame_rate,
      "video_device": device,
      "camera_name": "camera_front_link_optical",
      "camera_info_url": "package://camera_360/config/camera_info_front.yaml",
      "pixel_format": "mjpeg2rgb",
    }]
  )

  camera_left = Node(
    package="v4l2_camera",
    executable="v4l2_camera_node",
    parameters=[{
      "image_size": [1280, 720],
      "pixel_format": "YUYV",
      "video_device": "/dev/video1",
      "framerate": 30,
      "camera_frame_id": "camera_left_link_optical",
    }]
  )

  camera_right = Node(
    package="v4l2_camera",
    executable="v4l2_camera_node",
    parameters=[{
      "image_size": [1280, 720],
      "pixel_format": "mjpeg2rgb",
      "video_device": "/dev/video2",
      "framerate": 30,
      "camera_frame_id": "camera_right_link_optical",
    }]
  )

  nodes = [   
    camera_front,
    camera_left,
    camera_right,
  ]

  return LaunchDescription( environment_variable + nodes)



dev = scan_for_camera_devices()
for i, device in enumerate(dev):
    print(f"Device {i}: {device['device']} - {device['name']}")
    if i == 0:
        device_name = device['device']
        break