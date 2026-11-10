# Mission and auxiliary packages (`nomad_core/`)

Smaller packages for competition tasks (radiation mapping, sign detection),
the arm, visual odometry and comfort features. Each section states what the
code does now.

## Radiation mapping

- Sensor: the Geiger counter on CAN (`geiger_hardware`) and
  `geiger_broadcaster`, which publishes `geiger_msg/Radiation` (`siwert`, `cpm`).
  It is disabled in bringup. See [can-and-hardware-interfaces.md](can-and-hardware-interfaces.md).
- `nomad_radiation/radiation_map.py`: subscribes to `/radiation` and
  publishes `nav_msgs/OccupancyGrid` on `/radiation_heatmap`. Params:
  `resolution 0.2` m, `max_size_x 200`, `max_size_y 200` cells. Config in
  `config/radiation_map_params.yaml`, launch `launch/mapper.launch.py`.
- `elrob_signs/radiation_maper.py`: another mapper. It subscribes to
  `/geiger_state_broadcaster/radiation` and `/map` and publishes every 1 s.

The two mappers read different input topics. Neither one is in a default launch.

## Sign detection (`elrob_signs`)

`elrob_signs/detection.py` subscribes to `/image_raw`, `/camera_info` and
`/scan`. It publishes `/detected_sign/image` (Image) and
`/detected_sign/position` (PointStamped). It uses OpenCV with `cv_bridge`
and can send Nav2 `NavigateToPose` goals. Launch: `launch/detection.launch.py`.
The package name suggests it is for the ELROB competition. Not confirmed.

## Arm (`arm_config`)

A MoveIt Setup Assistant package for `nomad_robot` with planning group
`arm`. It contains the SRDF, kinematics, joint limits, Pilz cartesian
limits and `ros2_controllers.yaml`, plus launches `demo.launch.py`,
`move_group.launch.py` and others. The arm joints are `revolute_arm_1` to
`revolute_arm_6` in the Nomad-T URDF.

## Visual odometry (ORB-SLAM3)

- `dependencies/ORB-SLAM3_Ubuntu24.04` (library) is built by `local_build.sh`.
- `nomad_core/ROS2_ORB-SLAM3_Odometry` (submodule) is the ROS 2 wrapper,
  with `orbslam3_ros2`, `vision_opencv` and `image_transport_tutorials`.
- It is not part of the EKF yet. The road map plans `/visual_odom` as an EKF input.

## Comfort (`comfort_package`)

`audio_node.py` subscribes to `cmd_vel` (TwistStamped) and plays a
synthesised electric-motor sound (`electric_motor_synthesizer.py`). The
sound scales with velocity up to `max_velocity` (2.5 m/s). Sample rate is
48000 Hz. Launch: `launch/comfort.launch.py`.

## Shared types

- `shared_types_nomad`: `status.hpp` (a Status/Result type).
- `ros_ari_shared_types` (submodule, package `ari_shared_types`): used by
  `vesc_hardware`, which provides macros such as `ARI_ASIGN_OR_RETURN`.
