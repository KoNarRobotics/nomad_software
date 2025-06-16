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

# Condif base on package usb_cam
# https://github.com/ros-drivers/usb_cam

def generate_launch_description():
  # max values are 3840 x 2160 and 30.0 fps
  frame_rate = 15.0
  width = 1280
  height = 720
  focus = 0

  environment_variable = [
    # SetEnvironmentVariable(name="FASTDDS_BUILTIN_TRANSPORTS",value="LARGE_DATA")
    ]

  device = "/dev/video2"
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
      "autofocus" : False,
      "focus" : focus ,
      "autoexposure": True,
      "auto_white_balance": True,
    }]
  )

  device = "/dev/video0"
  camera_left = Node(
    package="usb_cam",
    executable="usb_cam_node_exe",
    namespace="camera_left",
    parameters=[{
      "image_width": width,
      "image_height": height,
      "framerate": frame_rate,
      "video_device": device,
      "camera_name": "camera_left_link_optical",
      "camera_info_url": "package://camera_360/config/camera_info_left.yaml",
      "pixel_format": "mjpeg2rgb",
      "autofocus" : False,
      "focus" : focus ,
      "autoexposure": True,
      "auto_white_balance": True,
    }]
  )

  device = "/dev/video4"
  camera_right = Node(
    package="usb_cam",
    executable="usb_cam_node_exe",
    namespace="camera_right",
    parameters=[{
      "image_width": width,
      "image_height": height,
      "framerate": frame_rate,
      "video_device": device,
      "camera_name": "camera_right_link_optical",
      "camera_info_url": "package://camera_360/config/camera_info_right.yaml",
      "pixel_format": "mjpeg2rgb",
      "autofocus" : False,
      "focus" : focus ,
      "autoexposure": True,
      "auto_white_balance": True,
    }]
  )

  nodes = [   
    camera_front,
    camera_left,
    camera_right,
  ]

  return LaunchDescription( environment_variable + nodes)
