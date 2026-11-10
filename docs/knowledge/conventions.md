# Units, frames and naming (`nomad_bringup/description/urdf/nomad.urdf.xacro`)

Every other page uses the units and frames defined here. The rover runs
ROS 2 Jazzy, so the ROS conventions (REP 103 and REP 105) apply unless a
page says otherwise.

## Units

- SI units: metres, seconds, radians, m/s, rad/s, N·m, A.
- Exceptions written in the code:
  - VESC position commands are sent in milli-degrees (`ros_vesc_hardware/src/vesc_hardware.cpp`).
  - The VESC reports electrical RPM (ERPM). The driver converts ERPM to rad/s
    by dividing by `gear_ratio * polar_pairs`. See [can-and-hardware-interfaces.md](can-and-hardware-interfaces.md#vesc-motor-controllers).
  - The Geiger state interface is named `siwert` (sievert) plus `cpm` (counts per minute).
- Dates in `docs/` are YYYY-MM-DD.

## Coordinate frames

The axes follow REP 103: x forward, y left, z up. Optical frames
(`*_link_optical`) use the camera convention: z forward, x right, y down.

TF tree of the real rover (Nomad-T), from `nomad.urdf.xacro` and
`camera_360/urdf/camera_360.xacro`:

```
map                         published by slam_toolbox
└─ odom                     published by robot_localization EKF (publish_tf: true)
   └─ base_link
      └─ base_reasonable_link      xyz -0.345 -0.2 0
         ├─ robot_link … (Nomad-T model from nomad_t_model.xacro: wheels,
         │                arm axis1_1…axis6_1, imu_link, gps, base_footprint)
         │     └─ imu_link        xyz 0.4 0.05 0.15, rpy 0 π/2 0
         └─ camera_360_mount       xyz 0.55 0.2 0.25
            ├─ camera_front_link ─ camera_front_link_optical
            ├─ camera_left_link  ─ camera_left_link_optical
            ├─ camera_right_link ─ camera_right_link_optical
            ├─ lidar_link (z +0.08) ─ base_laser
            └─ livox_frame (pitch 0.1745 rad = 10°)
```

- `nomad_diffdrive_base_controller` has `enable_odom_tf: false`, so only the
  EKF publishes `odom → base_link`.
- `base_reasonable_link` exists because the Nomad-T mesh origin is not at the
  robot centre. The offset `-0.345 -0.2 0` moves the model so that
  `base_link` is roughly at its centre. This is inferred from the name and the
  numbers and has not been measured.

The simulated Nomad-R uses its own tree. See [simulation-isaac.md](simulation-isaac.md#robot-model).

## Robot names

- **Nomad-T**: the current physical rover. Skid-steer with 4 VESC-driven
  wheels and a 6-axis arm (`axis1`…`axis6` meshes). Described by
  `nomad_simulation/nomad_t_model_description`.
- **Nomad-R**: the newer model with 4 steerable wheels and left/right rocker
  suspension. It currently exists in Isaac Sim (`nomad_simulation/nomad_r.usd`,
  `nomad_r_urdf`).
- **KonARM / sdrac**: the separate arm controller at `192.168.20.10`. Joystick
  input is switched to it on `/sdrac/joy`. See [teleoperation.md](teleoperation.md).

`nomad_simulation/nomad_complete.urdf` is the expanded xacro output of this
whole tree (last regenerated 2026-04-02, commit `160eb60`). It is the input
for the Isaac Sim import in `nomad_simulation/nomad_complete/`.

## Known limitations (as of 2026-11-10)
- `imu_link` is mounted with pitch π/2 (`rpy 0 1.5707963267948966 0`). The
  mounting has not been checked against the physical IMU.
