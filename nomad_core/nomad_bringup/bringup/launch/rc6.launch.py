
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration 
from launch.actions import SetEnvironmentVariable

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os


def generate_launch_description():
  environment_variable = [
    # SetEnvironmentVariable(name="FASTDDS_BUILTIN_TRANSPORTS",value="LARGE_DATA")
    ]

  pilot_6_axis = Node(
    package='pilot_6axis',
    executable='rc_node',
    name='rc_node',
    parameters=[
      {"pilot_six_axis_joy_topic" : "/joy" }
    ]
  )

  nodes = [
    pilot_6_axis
  ]

  return LaunchDescription(environment_variable + nodes)
