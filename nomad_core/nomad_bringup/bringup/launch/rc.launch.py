
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

  simple_control_switch = Node(
    package='simple_driver',
    executable='rc_switch',
    name='rc_switch',
  )

  rc_driver = Node(
    package='simple_driver',
    executable='rc_driver',
    name='rc_driver',
    remappings=[
      ('/joy', '/nomad/joy')
    ],
    parameters=[
      {"kaganiec_predkosci" : 0.2},
    ]
  )

  nodes = [
    pilot_6_axis,
    simple_control_switch,
    rc_driver
  ]

  return LaunchDescription(environment_variable + nodes)
