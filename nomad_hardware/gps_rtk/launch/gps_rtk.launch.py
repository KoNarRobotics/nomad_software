from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution, LaunchConfiguration
from launch.actions import SetEnvironmentVariable

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.launch_description_sources import PythonLaunchDescriptionSource
import ament_index_python.packages

import os
import subprocess


def generate_launch_description():


  environment_variable = [
    SetEnvironmentVariable(
        name='NTRIP_CLIENT_DEBUG',  # Name of the environment variable
        value='false'               # Disable debug output
    )
    # SetEnvironmentVariable(name="FASTDDS_BUILTIN_TRANSPORTS",value="LARGE_DATA")
    ]
  
  config_directory = os.path.join( ament_index_python.packages.get_package_share_directory('gps_rtk'),
    'config')
  params = os.path.join(config_directory, 'zed_f9p.yaml')
  ublox_gps_node = Node(package='ublox_gps',
                                            executable='ublox_gps_node',
                                            output='both',
                                            parameters=[ os.path.join(config_directory, 'zed_f9p.yaml')])

  # ******************************************************************
  # NTRIP Client Node
  # ******************************************************************
  ntrip_node = Node(
        package='ntrip_client',              # ROS 2 package containing the NTRIP client
        executable='ntrip_ros.py',           # Python script for the NTRIP client
        name='ntrip_client',                 # Name assigned to the node
        output='screen',                     # Output log to screen
        parameters=[ os.path.join(config_directory, 'ntrip_config.yaml')],
        remappings=[
            ('/fix', '/ublox_gps_node/fix')  # Remap /fix topic to /ublox_gps_node/fix
        ]
    )
  nodes = [   
    ublox_gps_node,
    ntrip_node
  ]

  return LaunchDescription( environment_variable + nodes)
