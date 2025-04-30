from launch import LaunchDescription
from launch_ros.actions import Node
import os

def resolve_relative_path(relative_path):
  return os.path.abspath(os.path.join(os.path.dirname(__file__), relative_path))

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='pilot_6axis',
            executable='rc_node',
            name='rc',
            parameters=[
                {"pilot_six_axis_joy_topic" : "/joy" }
            ]
        ),
        Node(
            package='simple_driver',
            executable='sd_node',
            name='simple_driver',
            parameters=[
                {
                  "can_db" : resolve_relative_path("../nomad_hardware/can_device/can_constants/can_messages/output/can.dbc") ,
                  "kaganiec_predkosci" : 0.3
                }
            ],
            remappings=[
                ("/joy", "/nomad/joy")
            ]
        ),
          Node(
            package='simple_driver',
            executable='rc_switch',
            name='rc_switch'
        )
    ])
