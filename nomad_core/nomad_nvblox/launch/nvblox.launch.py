# SPDX-FileCopyrightText: NVIDIA CORPORATION & AFFILIATES
# Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

import isaac_ros_launch_utils as lu
from launch import LaunchDescription
from launch_ros.actions import Node, SetParameter
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    # Declare launch arguments
    depth_camera_input = DeclareLaunchArgument(
        'depth_camera_input',
        default_value='/nvblox/depth/image_raw',
        description='Depth camera image input topic'
    )

    depth_camera_info = DeclareLaunchArgument(
        'depth_camera_info',
        default_value='/nvblox/depth/camera_info',
        description='Depth camera info input topic'
    )

    color_camera_input = DeclareLaunchArgument(
        'color_camera_input',
        default_value='/nvblox/camera/image_raw',
        description='Color camera image input topic'
    )

    color_camera_info = DeclareLaunchArgument(
        'color_camera_info',
        default_value='/nvblox/camera/camera_info',
        description='Color camera info input topic'
    )



    camera_mask = DeclareLaunchArgument(
        'camera_mask',
        default_value='/nvblox/mask/image_raw',
        description='Color camera image input topic'
    )

    camera_mask_info = DeclareLaunchArgument(
        'camera_mask_info',
        default_value='/nvblox/mask/camera_info',
        description='Color camera info input topic'
    )


    lidar_input = DeclareLaunchArgument(
        'lidar_input',
        default_value='/nvblox/lidar/point_cloud',
        description='Lidar point cloud input topic'
    )

    mode = DeclareLaunchArgument(
        'mode',
        default_value='dynamic',
        description='Nvblox mode (static, dynamic)'
    )

    # Set use_sim_time parameter globally
    set_use_sim_time = SetParameter('use_sim_time', True)

    # Get package-based paths for configuration files
    base_config = lu.get_path('nomad_nvblox', 'config/nvblox_base.yaml')
    dynamics_config = lu.get_path('nomad_nvblox', 'config/nvblox_dynamics.yaml')

    # Launch nvblox node with appropriate configurations
    nvblox_node = Node(
        package='nvblox_ros',
        executable='nvblox_node',
        name='nvblox_node',
        output='screen',
        parameters=[
            {'use_sim_time': True},
            {'mode': LaunchConfiguration('mode')},
            # Load base configuration
            base_config,
            # Load mode-specific configuration (dynamics in this case)
            dynamics_config
        ],
        remappings=[
            ('camera_0/depth/image', LaunchConfiguration('depth_camera_input')),
            ('camera_0/depth/camera_info', LaunchConfiguration('depth_camera_info')),
            ('camera_0/color/image', LaunchConfiguration('color_camera_input')),
            ('camera_0/color/camera_info', LaunchConfiguration('color_camera_info')),
            ('camera_0/mask/image',LaunchConfiguration('camera_mask')),
            ('camera_0/mask/camera_info',LaunchConfiguration('camera_mask_info')),
            ('pointcloud', LaunchConfiguration('lidar_input')),
        ]
    )

    # Component container for nvblox
    container = Node(
        package='rclcpp_components',
        executable='component_container_mt',
        name='nvblox_container',
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    # Add color camera remappings (for RGB-D cameras)
    # Note: In a real RGB-D setup, these would be properly mapped from the sensor topics
    # For now, we're setting up the launch file structure to support them
    return LaunchDescription([
        depth_camera_input,
        depth_camera_info,
        color_camera_input,
        color_camera_info,
        camera_mask,
        camera_mask_info,
        lidar_input,
        mode,
        set_use_sim_time,
        nvblox_node,
        container
    ])
