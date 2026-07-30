# Copyright 2026 Nomad Robotics
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Launches the suspension_driver swerve-drive node.

Usage:
    ros2 launch suspension_driver suspension_driver.launch.py
    ros2 launch suspension_driver suspension_driver.launch.py params_file:=/path/to/overrides.yaml
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:
    default_params_file = os.path.join(
        get_package_share_directory('suspension_driver'),
        'config',
        'suspension_driver.yaml',
    )

    params_file_arg = DeclareLaunchArgument(
        'params_file',
        default_value=default_params_file,
        description='Full path to the parameter YAML for suspension_driver.',
    )

    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation clock (set true when driving Isaac Sim / Gazebo).',
    )

    suspension_driver_node = Node(
        package='suspension_driver',
        executable='suspension_driver_node',
        name='suspension_driver',
        output='screen',
        emulate_tty=True,
        parameters=[
            LaunchConfiguration('params_file'),
            {'use_sim_time': LaunchConfiguration('use_sim_time')},
        ],
    )

    return LaunchDescription([
        params_file_arg,
        use_sim_time_arg,
        suspension_driver_node,
    ])
