from launch import LaunchDescription
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration
from launch.actions import SetEnvironmentVariable
from launch.actions import DeclareLaunchArgument, RegisterEventHandler, IncludeLaunchDescription

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource

from ament_index_python.packages import get_package_share_directory

import os
import subprocess


# BASE ON 
# https://github.com/methylDragon/ros-sensor-fusion-tutorial/blob/master/01%20-%20ROS%20and%20Sensor%20Fusion%20Tutorial.md

def generate_launch_description():

  environment_variable = [
    # SetEnvironmentVariable(name="FASTDDS_BUILTIN_TRANSPORTS",value="LARGE_DATA")
    ]

  front_cam_node =  Node(
    package='elrob_signs',
    executable='detection_node',
    name='detection_node',
    output='screen',
    remappings=[
      ('/camera_info', '/camera_front/camera_info'),
      ('/image_raw', '/camera_front/image_raw'),
    ]
  )

  right_cam_node =  Node(
    package='elrob_signs',
    executable='detection_node',
    name='detection_node',
    output='screen',
    remappings=[
      ('/camera_info', '/camera_right/camera_info'),
      ('/image_raw', '/camera_right/image_raw'),
    ]
  )

  left_cam_node =  Node(
    package='elrob_signs',
    executable='detection',
    name='detection_node',
    output='screen',
    remappings=[
      ('/camera_info', '/camera_left/camera_info'),
      ('/image_raw', '/camera_left/image_raw'),
    ]
  )

  nodes = [
    # front_cam_node,
    right_cam_node,
    left_cam_node
  ]

  return LaunchDescription( environment_variable + nodes)
