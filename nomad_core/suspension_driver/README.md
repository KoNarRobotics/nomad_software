# suspension_driver

ROS 2 (Jazzy/Humble) driver for a 4-wheel independent-drive, independent-steering
(swerve) mobile base. Converts `/cmd_vel` into per-wheel drive velocity and
steering position commands (inverse kinematics) and fuses per-wheel feedback
into `/wheel_odom` (least-squares forward kinematics + exact pose integration).

Kinematics reference:
[control.ros.org - mobile robot kinematics](https://control.ros.org/humble/doc/ros2_controllers/doc/mobile_robot_kinematics.html)

## Architecture

- `swerve_kinematics` - pure C++20 library (no ROS dependencies), unit-tested.
- `suspension_driver_node` - ROS 2 wrapper: topics, parameters, watchdog, TF.

Wheel arrays are always ordered **FL, FR, RL, RR**. Frames follow REP-103
(x forward, y left, CCW-positive yaw); steering angle 0 points along +x.

## Topics

| Topic | Type | Direction | Description |
|---|---|---|---|
| `/cmd_vel` | `geometry_msgs/Twist` | sub | Target body twist (vx, vy, wz) |
| `/wheel_velocity` | `std_msgs/Float64MultiArray` | sub | 4x drive wheel velocity [rad/s] |
| `/wheel_pos` | `std_msgs/Float64MultiArray` | sub | 4x drive wheel position [rad] |
| `/wheel_rotation_pos` | `std_msgs/Float64MultiArray` | sub | 4x steering angle [rad] |
| `/drive_velocity_cmd` | `std_msgs/Float64MultiArray` | pub | 4x drive velocity target [rad/s] |
| `/steering_position_cmd` | `std_msgs/Float64MultiArray` | pub | 4x steering angle target [rad] |
| `/wheel_odom` | `nav_msgs/Odometry` | pub | Integrated wheel odometry |

## Parameters

See `config/suspension_driver.yaml`: `wheel_radius`, `wheel_base`,
`track_width`, `control_rate`, `max_drive_velocity`, `cmd_vel_timeout`,
`publish_tf`, `odom_frame_id`, `base_frame_id`.

## Build & run

```bash
colcon build --packages-select suspension_driver
colcon test  --packages-select suspension_driver   # kinematics unit tests
source install/setup.bash
ros2 launch suspension_driver suspension_driver.launch.py use_sim_time:=true
```

## Safety behavior

- Watchdog: if `/cmd_vel` is older than `cmd_vel_timeout`, zero drive
  velocities are published while the current steering angles are held.
- Steering optimization: modules never steer more than 90 deg; the drive
  direction is reversed instead.
- Malformed feedback arrays (size != 4) are rejected and logged (throttled).

For running the base in NVIDIA Isaac Sim see `docs/isaac_sim_guide.md`.
