# Simulation (`nomad_simulation/`)

Robot models for visualisation and simulation. Nomad-R is simulated in
NVIDIA Isaac Sim. Nomad-T has an older Gazebo setup and, since 2026-11-06,
an uncommitted Isaac Sim import.

## Robot model

| Asset | What it is |
|---|---|
| `nomad_r.usd` (≈76 MB, committed as a binary) | Isaac Sim stage of Nomad-R with sensors and the ROS 2 OmniGraph. |
| `nomad_r_urdf/` | Nomad-R URDF (`urdf/nomad_r.urdf`, 11 links) and STL meshes. `launch/display.launch.py` shows it in RViz. |
| `nomad_t_model_description/` | Nomad-T xacro with meshes, `display.launch.py` and `gazebo.launch.py`. |
| `nomad_complete.urdf` | Expanded Nomad-T URDF (see [conventions.md](conventions.md#coordinate-frames)). |
| `nomad_complete/` (untracked) | `nomad_complete.usda` with payloads and textures, made by "URDF USD Converter v0.1.3" from `nomad_complete.urdf`. It has physics variants `physx`, `mujoco` and `none`. |
| `luxonis_oak_d_lite/` | OAK-D Lite camera USD and STL. |
| `isaac/livox_mid_360.conf`, `isaac/set_lidar_mod360_prop.py` | MID-360 RTX lidar settings: rotary, 40 channels, 0.1 to 40 m, 20 Hz scan rate, from an NVIDIA forum post. The script recreates the prim at `/nomad_r_urdf/base_view/sensors_mount/lidar_livox_Mid360`. |

Nomad-R joints (all `revolute`, from `nomad_r.urdf`):
- suspension: `right_side_suspension_joint`, `left_side_suspension_joint`
- steering: `wheel_rot_{right,left}_{front,rear}_joint`
- drive: `wheel_{right,left}_{front,rear}_joint`

Sensors in the Nomad-R stage (from commit history, 2026-05 to 2026-07):
IMU, cameras, a stereo camera with TF, and the Livox lidar. Commit
`405bf1a` (2026-06-14) says driving, cameras, IMU and lidar work.

## Running the simulation

From the README:

```bash
source setup_local_env.sh
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run remote_6d remote_6d_node        # terminal 1: Pilot remote → /joy
ros2 run simple_driver diff_drive_fix    # terminal 2: /joy → /joint_command
# then open Isaac Sim, load nomad_r.usd and a world, press Play
```

## Driving the simulated rover

`simple_driver/diff_drive_fix.py` (node name `simple_driver`):
- subscribes to `/joy` and publishes `sensor_msgs/JointState` on `/joint_command`, `frame_id nomad_r`
- drive wheel velocity: `axes[0] · 20` (rad/s). Right wheels get `+v`, left wheels get `−v`.
- steering: all four steering joints get the same position `turn_val`.
  `turn_val` integrates `axes[2] · 0.04` on every message, so steering
  speed depends on the `/joy` rate.
- the suspension joints are always commanded to position 0.

## Known limitations (as of 2026-11-10)
- Steering is the same angle on all four wheels (crab steering). There is no
  Ackermann or rotate-in-place kinematics yet. Planned in
  [proposal 0001](../proposals/0001-autonomy-pipeline.md).
- `nomad_r.usd` has uncommitted changes, and `nomad_complete/` is untracked
  (git status on 2026-11-10).
- Large binary USD files are committed without Git LFS. See Q9.
