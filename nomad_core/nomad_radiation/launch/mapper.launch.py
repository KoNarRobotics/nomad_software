from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration
from launch.actions import SetEnvironmentVariable

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


  radiation_mapper_node =  Node(
    package='nomad_radiation',
    executable='radiation_map_node',
    name='radiation_map_node',
    output='screen',
    parameters=[os.path.join(get_package_share_directory("nomad_radiation"), 'config', 'radiation_map_params.yaml')],
    remappings=[
      ('/radiation', '/geiger_state_broadcaster/radiation'),
      # ('/radiation_heatmap', '/radiation_heatmap'),
    ]
  )

  nodes = [
    radiation_mapper_node
  ]

  return LaunchDescription( environment_variable + nodes)
