# Localization and navigation (`nomad_core/nomad_sensor_fusion`, `nomad_core/nomad_navigation`)

On the rover, the EKF fuses wheel odometry and the IMU into `odom → base_link`.
slam_toolbox builds a 2D map from the lidar `/scan` and publishes
`map → odom`, and Nav2 plans and drives on that map. Frames are defined in
[conventions.md](conventions.md#coordinate-frames).

## EKF (`nomad_sensor_fusion`)

`launch/fusion.launch.py` runs `robot_localization ekf_node` with
`config/ekf.yaml`. It is included from `diffbot.launch.py`.

- Remaps: `/odom → /odom_in` and `/odometry/filtered → /odom`. The fused
  output is `/odom`, which Nav2 reads.
- `frequency: 15.0` Hz, `sensor_timeout: 0.2` s, `two_d_mode: true`,
  `publish_tf: true`, `world_frame: odom`.
- `odom0: nomad_diffdrive_base_controller/odom_fix` fuses x, y and yaw.
  `odom0_relative: true`, pose rejection 5.0, twist rejection 1.0.
- `imu0: imu_state_broadcaster/imu` fuses roll, pitch and yaw plus the three
  angular rates. `imu0_relative: true`, rejection thresholds 0.8, gravity
  not removed.
- `use_control: false`. The control config is present but unused.
- `debug_out_file` is set to an absolute path under `/home/ubuntu/...`, and `debug: false`.
- `nomad_sensor_fusion/sensor_fusion.py` is empty.

`launch/navsat_fusion.launch.py` runs `navsat_transform_node` with
`/imu → /imu_state_broadcaster/imu`, `/gps/fix → /ublox_gps_node/fix` and
`/odometry/filtered → /odom`. No default launch includes it.

## Mapping: slam_toolbox (`nomad_navigation/launch/slam.launch.py`)

- `async_slam_toolbox_node` with `config/mapper.yaml`.
- `mode: mapping` (the comment shows `localization` as the alternative),
  `scan_topic: /scan`, `resolution: 0.05` m, `max_laser_range: 40.0` m.
- `minimum_travel_distance: 0.5` m, `minimum_travel_heading: 0.5` rad.
- Frames: `map`, `odom`, `base_link`.

## Nav2 (`nomad_navigation/launch/nav2.launch.py`, `config/nav2.yaml`)

`navigation.launch.py` includes `nav2.launch.py` and `slam.launch.py`. The
frontier explorer (`explore_lite`, from the
`Autonomous-Explorer-and-Mapper-ros2-nav2` submodule) is commented out.
`nomad_nav.service` starts `navigation.launch.py`.

Servers: controller, smoother, planner, behaviors, bt_navigator,
waypoint_follower, velocity_smoother, collision_monitor. Docking is
commented out. All use `enable_stamped_cmd_vel: true`.

Velocity chain (the standard Nav2 layout):

```
controller_server / behavior_server → cmd_vel_nav → velocity_smoother → cmd_vel_smoothed
  → collision_monitor → /cmd_vel → nomad_diffdrive_base_controller
```

| Component | Setting |
|---|---|
| Controller | `nav2_mppi_controller::MPPIController`, `motion_model: DiffDrive`, `controller_frequency: 10.0` Hz, `time_steps 15`, `model_dt 0.1`, `batch_size 2000`, `vx_max 0.4`, `vx_min −1.0`, `wz_max 1.9` |
| Planner | `nav2_navfn_planner::NavfnPlanner` |
| Smoother | `nav2_smoother::SimpleSmoother` |
| Velocity smoother | `max_velocity [0.7, 0.0, 0.8]`, `min_velocity [−0.7, 0.0, −0.8]` |
| Local costmap | `odom` frame, 0.05 m, voxel and inflation layers, source `/scan` (LaserScan), footprint 1.30 × 0.74 m (`±0.65, ±0.37`) |
| Global costmap | `map` frame, 0.05 m, static, obstacle and inflation layers, source `/scan`, footprint 0.70 × 0.60 m (`±0.35, ±0.30`) |
| Collision monitor | `cmd_vel_smoothed → cmd_vel`, source `scan`, uses `/local_costmap/published_footprint` |

The AMCL section uses `base_frame_id: base_footprint`, but AMCL is not started (slam_toolbox is used instead).

Rover teleop can cancel a Nav2 goal with button 5. See [teleoperation.md](teleoperation.md#rc_driver-simple_driverrc_driverpy).

## Status

- 2025-06-23 (commit `41b1c4f`): maps, local and global costmaps and SLAM
  worked, but the rover did not reach goals because the commanded velocity
  was too low.
- 2025-11-21 (commit `5f5677c`): "finally works sensor fusion".

## Known limitations (as of 2026-11-10)
- The EKF's `odom0` input `nomad_diffdrive_base_controller/odom_fix` is no
  longer published, so the EKF runs on the IMU alone. See
  [rover-bringup.md](rover-bringup.md#known-limitations-as-of-2026-11-10) and Q2.
  The real wheel odometry is on `/nomad_diffdrive_base_controller/odom`.
- The local and global costmap footprints differ by almost a factor of two
  in each direction. Neither has been checked against the real robot. See Q7.
- The MPPI `vx_max` of 0.4 m/s is far below the 3.5 m/s drive limit and the
  0.7 m/s velocity-smoother limit.
