from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='simple_driver',
            executable='diff_drive_fix.py',
            name='simple_driver_node',
            output='screen'
        )
    ])