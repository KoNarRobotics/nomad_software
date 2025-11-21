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

def generate_launch_description():

  environment_variable = [
    # SetEnvironmentVariable(name="FASTDDS_BUILTIN_TRANSPORTS",value="LARGE_DATA")
    ]

  audio_node =  Node(
    package='comfort_package',
    executable='audio_node',
    name='audio_node',
    output='screen',
    parameters=[PathJoinSubstitution([FindPackageShare('comfort_package'), 'config', 'params.yaml'])],
  )

  nodes = [
    audio_node
  ]

  return LaunchDescription( environment_variable + nodes)
