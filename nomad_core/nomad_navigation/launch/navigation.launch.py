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


  nav_launch = IncludeLaunchDescription(
    PythonLaunchDescriptionSource([
      PathJoinSubstitution([
        FindPackageShare("nav2_bringup"),
        "launch",
        "nav.launch.py",
      ])
    ])
  )

  slam_launch = IncludeLaunchDescription(
    PythonLaunchDescriptionSource([
      PathJoinSubstitution([
        FindPackageShare("nomad_navigation"),
        "launch",
        "slam.launch.py",
      ])
    ])
  )

  nodes = [
    nav_launch,
    slam_launch,
  ]

  return LaunchDescription( environment_variable + nodes)
