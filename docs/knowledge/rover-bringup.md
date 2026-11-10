# Rover bringup (`nomad_core/nomad_bringup`)

The main launch for the physical rover (Nomad-T). It starts
`ros2_control`, the drive controller, the sensor broadcasters, cameras,
lidar, the EKF and remote control. The `nomad_base.service` systemd unit
runs it through `services/start_base.sh`.

## Launch: `ros2 launch nomad_bringup diffbot.launch.py`

Arguments:
- `gui` (default `true`): start RViz with `nomad_t_model_description/config/display.rviz`.
- `use_mock_hardware` (default `false`): passed to xacro. `nomad.urdf.xacro`
  currently declares no argument with that name and does not use it.

What it starts:

| Item | Source |
|---|---|
| `ros2_control_node` | params `bringup/config/nomad_controllers.yaml` |
| `robot_state_publisher` | `description/urdf/nomad.urdf.xacro` |
| `nomad_diffdrive_base_controller` spawner | `/nomad_diffdrive_base_controller/cmd_vel` remapped to `/cmd_vel` |
| `joint_state_broadcaster` | spawned after the drive controller spawner exits |
| RViz | started after `joint_state_broadcaster` spawner exits, if `gui` |
| `barometer_state_broadcaster`, `imu_state_broadcaster`, `gps_state_broadcaster` | spawners |
| `simple_driver diff_drive_fix` | see [the limitation below](#known-limitations-as-of-2026-11-10) |
| `camera_360/launch/camera.launch.py`, `camera_360/launch/lidar.launch.py` | [sensors-lidar-and-cameras.md](sensors-lidar-and-cameras.md) |
| `nomad_sensor_fusion/launch/fusion.launch.py` | [localization-and-navigation.md](localization-and-navigation.md#ekf-nomad_sensor_fusion) |
| `nomad_bringup/launch/rc.launch.py` | [teleoperation.md](teleoperation.md) |

The Geiger broadcaster is commented out in the launch file, the controller
YAML and the URDF.

## Controller manager (`bringup/config/nomad_controllers.yaml`)

- `update_rate: 20` Hz.
- `nomad_diffdrive_base_controller` (`diff_drive_controller/DiffDriveController`):
  - wheels: left `left_wheel_joint`, `left_wheel_joint_2`; right
    `right_wheel_joint`, `right_wheel_joint_2`.
  - `wheel_separation: 0.64` m, `wheel_radius: 0.13` m.
  - `wheel_separation_multiplier: 3.5`. The comment says the robot turned
    almost twice per real turn at 1.0, while forward distance was correct.
    The value was tuned by hand and depends on the ground surface.
  - `publish_rate: 15.0` Hz, `cmd_vel_timeout: 0.5` s, `position_feedback: true`, `open_loop: false`.
  - limits: `linear.x` ±3.5 m/s, `angular.z` ±5.0 rad/s. Acceleration limits are not set.
  - `enable_odom_tf: false`. The EKF publishes `odom → base_link`.
- Sensor broadcasters: `barometer_state_broadcaster` (frame `base_link`),
  `imu_state_broadcaster` (frame `imu_link`), `gps_state_broadcaster`
  (frame `base_link`, `read_covariance_from_interface: true`). All static
  covariances are zero.

## ros2_control hardware (`description/ros2_control/*.xacro`)

All plugins talk over `can0`. Details in [can-and-hardware-interfaces.md](can-and-hardware-interfaces.md).

| xacro | Plugin | Interfaces |
|---|---|---|
| `ros_control_vesc.xacro` | `vesc_hardware/MCVescHardware` (system) | 4 wheel joints. Commands: velocity ±40, current ±60, effort ±7. States: position, velocity, current, effort. |
| `ros_control_imu.xacro` | `imu_hardware/MCImuHardware` | orientation xyzw, linear_acceleration, magnetic_field, angular_velocity, temperature |
| `ros_control_gps.xacro` | `gps_hardware/MCGpsHardware` | status, service, lat/lon/alt plus their covariances, date and time fields |
| `ros_control_barometer.xacro` | `barometer_hardware/MCBarometerHardware` | temperature, pressure |
| `ros_control_geiger.xacro` (not included) | `geiger_hardware/MCGeigerHardware` | `siwert`, `cpm` |

The VESC xacro sets `command_interface_type: velocity`, `gear_ratio: 4.375`,
`polar_pairs: 7`, `current_to_torque: 0.0665` (N·m/A).

## Other launch files

- `rc.launch.py`: Pilot 6-axis remote, `rc_switch` and `rc_driver`. See [teleoperation.md](teleoperation.md).
- `rc6.launch.py`: only the Pilot 6-axis remote node.
- `rviz.launch.py`, `description/launch/view_robot.launch.py`: visualisation.
- `bringup/config/mc_config.yaml`: config for the ModuCard plugin driver. See
  [can-and-hardware-interfaces.md](can-and-hardware-interfaces.md#moducard-mc-modules).

## Known limitations (as of 2026-11-10)
- **`diff_drive_fix` is no longer an odometry fix.** On 2026-06-09 (commit
  `337b501`), `simple_driver/diff_drive_fix.py` was rewritten into a joystick
  driver for the Isaac Sim Nomad-R. It subscribes to `/joy` and publishes
  `sensor_msgs/JointState` on `/joint_command`. `diffbot.launch.py` still
  starts it with remappings for `/odom_in` and `/odom_out`, and those
  remappings now do nothing. As a result, nothing publishes
  `/nomad_diffdrive_base_controller/odom_fix`, which is the EKF's `odom0`
  input. This was found by reading the code and has not been checked on the rover. See Q2.
- **The VESC xacro in `nomad_bringup` does not match the `ros_vesc_hardware`
  submodule.** The submodule's `init()` (commit `daeebea`, 2026-04-02) reads the
  `prefix`, `gear_ratios`, `polar_pairs`, `current_to_torque` and
  `command_interface_type` parameters, each as a comma-separated list with
  one entry per joint. It also reads `vesc_base_ids`. `nomad_bringup` passes
  the old single values, `gear_ratio` (singular), and no `prefix` or
  `vesc_base_ids`. `hardware_parameters.at("prefix")` throws when the key
  is missing. The submodule ships a matching xacro in
  `ros_vesc_hardware/description/ros2_control/ros_control_vesc.xacro`, with
  `vesc_base_ids` 20,21,22,23 and command type `effort`. This was found by
  reading the code and not tested. See Q3.
- `diff_drive_fix_launch.py` refers to executable `diff_drive_fix.py`. The
  console script is named `diff_drive_fix`.
