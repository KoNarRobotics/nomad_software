from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    orb_slam_dir = get_package_share_directory('orbslam3_ros2')

    orb_slam_node = Node(
        package="orbslam3_ros2",
        executable="stereo_inertial",
        name="orb_slam3_node",
        output="screen",
        arguments=[
                os.path.join(orb_slam_dir, 'config/vocabulary/ORBvoc.txt'),
                os.path.join(orb_slam_dir, 'config/stereo-inertial/EuRoC.yaml'),
                'false'
        ],
        remappings=[
            ('/camera/left/image_raw', '/stereo_image/left'),
            ('/camera/right/image_raw', '/stereo_image/right'),
            ('/imu', '/imu'),
            ('/odom', '/visual_odom')
        ]
    )

    return LaunchDescription([
        orb_slam_node
    ])