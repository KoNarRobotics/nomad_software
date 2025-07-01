
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
       # SetEnvironmentVariable(name="RMW_IMPLEMENTATION",value="rmw_zenoh_cpp")
      ]

    # Declare arguments
    declared_arguments = []
    declared_arguments.append(
        DeclareLaunchArgument(
            "gui",
            default_value="true",
            description="Start RViz2 automatically with this launch file.",
        )
    )
    declared_arguments.append(
        DeclareLaunchArgument(
            "use_mock_hardware",
            default_value="false",
            description="Start robot with mock hardware mirroring command to its states.",
        )
    )

    # Initialize Arguments
    gui = LaunchConfiguration("gui")
    use_mock_hardware = LaunchConfiguration("use_mock_hardware")

    # Get URDF via xacro
    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            PathJoinSubstitution(
                [FindPackageShare("nomad_bringup"), "urdf", "nomad.urdf.xacro"]
            ),
            " ",
            "use_mock_hardware:=",
            use_mock_hardware,
        ]
    )
    robot_description = {"robot_description": robot_description_content}

    robot_controllers = PathJoinSubstitution(
        [
            FindPackageShare("nomad_bringup"),
            "config",
            "nomad_controllers.yaml",
        ]
    )
    rviz_config_file = PathJoinSubstitution(
        [FindPackageShare("nomad_t_model_description"), "nomad_t_model_description/config", "display.rviz"]
    )

    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_controllers],
        output="both",
    )
    robot_state_pub_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="both",
        parameters=[robot_description],
    )
    
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="log",
        arguments=["-d", rviz_config_file],
        condition=IfCondition(gui),
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster"],
    )

    barometer_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["barometer_state_broadcaster"],
    )

    geiger_state_broadcaster_spawner = Node(
      package="controller_manager",
      executable="spawner",
      arguments=["geiger_state_broadcaster"],
    )

    imu_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["imu_state_broadcaster"],
      )

    gps_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["gps_state_broadcaster"],
    )

    robot_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "nomad_diffdrive_base_controller",
            "--param-file",
            robot_controllers,
            "--controller-ros-args",
            "-r /nomad_diffdrive_base_controller/cmd_vel:=/cmd_vel",
        ],
    )


    # cameras
    camera_launch = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([
          PathJoinSubstitution([
              FindPackageShare("camera_360"),
              "launch",
              "camera.launch.py",
          ])
      ]))
    
    livox_launch=   IncludeLaunchDescription(
      PythonLaunchDescriptionSource([
          PathJoinSubstitution([
              FindPackageShare("camera_360"),
              "launch",
              "lidar.launch.py",
          ])
      ]))
    
    # Sensor Fusion
    sensor_fusion_launch = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([
          PathJoinSubstitution([
              FindPackageShare("nomad_sensor_fusion"),
              "launch",
              "fusion.launch.py",
          ])
      ]))
    
        # Sensor Fusion
    rc_launch = IncludeLaunchDescription(
      PythonLaunchDescriptionSource([
        PathJoinSubstitution([
          FindPackageShare("nomad_bringup"),
          "launch",
          "rc.launch.py",
        ])
    ]))

    # Delay rviz start after `joint_state_broadcaster`
    delay_rviz_after_joint_state_broadcaster_spawner = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[rviz_node],
        )
    )

    # Delay start of joint_state_broadcaster after `robot_controller`
    # TODO(anyone): This is a workaround for flaky tests. Remove when fixed.
    delay_joint_state_broadcaster_after_robot_controller_spawner = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=robot_controller_spawner,
            on_exit=[joint_state_broadcaster_spawner],
        )
    )

    nodes = [
      control_node,
      robot_state_pub_node,
      robot_controller_spawner,
      # joint_state_broadcaster_spawner,
      delay_rviz_after_joint_state_broadcaster_spawner,
      delay_joint_state_broadcaster_after_robot_controller_spawner,
      barometer_state_broadcaster_spawner,
      imu_state_broadcaster_spawner,
      gps_state_broadcaster_spawner,
      geiger_state_broadcaster_spawner,
      camera_launch,
      livox_launch,
      sensor_fusion_launch,
      rc_launch,
    ]

    return LaunchDescription(environment_variable + declared_arguments + nodes)
