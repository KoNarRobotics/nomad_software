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
from launch.conditions import IfCondition
from launch.substitutions import PythonExpression
from launch_ros.actions import SetParameter

from nvblox_ros_python_utils.nvblox_constants import NVBLOX_CONTAINER_NAME
from nvblox_ros_python_utils.nvblox_launch_utils import (
    NvbloxCamera, NvbloxMode, NvbloxPeopleSegmentation)


def generate_launch_description() -> LaunchDescription:
    args = lu.ArgumentContainer()
    args.add_arg(
        'rosbag', 'None', description='Path to rosbag (running on sensor if not set).', cli=True)
    args.add_arg('rosbag_args', '',
                 description='Additional args for ros2 bag play.', cli=True)
    args.add_arg('log_level', 'info', choices=[
                 'debug', 'info', 'warn'], cli=True)
    args.add_arg('num_cameras', 1,
                 description='How many cameras to use.', cli=True)
    args.add_arg(
        'mode',
        default=NvbloxMode.static,
        choices=NvbloxMode.names(),
        description='The nvblox mode.',
        cli=True)
    args.add_arg(
        'attach_to_container',
        'False',
        description='Add components to an existing component container.',
        cli=True)
    args.add_arg(
        'container_name',
        NVBLOX_CONTAINER_NAME,
        description='Name of the component container.')
    args.add_arg(
        'use_foxglove_whitelist',
        True,
        description='Disable visualization of bandwidth-heavy topics',
        cli=True)
    args.add_arg(
        'camera_input_topic',
        '/nvblox/camera/image_raw',
        description='Input depth image topic name.',
        cli=True)
    args.add_arg(
        'camera_info_topic',
        '/nvblox/camera/camera_info',
        description='Camera info topic name.',
        cli=True)
    args.add_arg(
        'use_yaml_config',
        'True',
        description='Use YAML configuration file for nvblox parameters.',
        cli=True)
    actions = args.get_launch_actions()

    # Globally set use_sim_time if we're running from bag or sim
    actions.append(
        SetParameter('use_sim_time', True, condition=IfCondition(lu.is_valid(args.rosbag))))

    # Single camera mode
    camera_mode = str(NvbloxCamera.depth_camera)

    # Nvblox
    actions.append(
        lu.include(
            'nvblox_examples_bringup',
            'launch/perception/nvblox.launch.py',
            launch_arguments={
                'container_name': args.container_name,
                'mode': args.mode,
                'camera': camera_mode,
                'num_cameras': args.num_cameras,
                'camera_input_topic': args.camera_input_topic,
                'camera_info_topic': args.camera_info_topic,
                'use_yaml_config': args.use_yaml_config,
            }))

    # Container
    actions.append(
        lu.component_container(
            NVBLOX_CONTAINER_NAME, condition=UnlessCondition(args.attach_to_container),
            log_level=args.log_level))

    return LaunchDescription(actions)