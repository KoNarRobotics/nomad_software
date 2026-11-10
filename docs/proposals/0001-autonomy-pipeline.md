# 0001 Autonomy pipeline for Nomad-R

Status: draft · Date: 2026-11-10

Transcribed from the "ROAD MAP" and "Extended Read Map" sections of the root
`README.md` (last edited 2026-07-08). Owners are given as the README names them.

## Problem

Nomad-R has steerable wheels and rocker suspension and is meant to drive on
rough terrain. The current Nomad-T stack (2D slam_toolbox with Nav2 MPPI,
see [localization-and-navigation.md](../knowledge/localization-and-navigation.md))
has three gaps: it assumes flat ground and a 2D laser scan, it uses
diff-drive kinematics, and it does not tell static obstacles from dynamic ones.

## Proposal

A pipeline in four layers.

### Sensors
| Node | Outputs | Owner |
|---|---|---|
| `depth_camera_node` | `/stereo_image`, `/depth_image`, `/rgb_image` | |
| `lidar_driver_node` | `/point_cloud2` | |
| `imu_sensor_node` | `/imu` | |
| `suspension_driver` | `/wheel_odom`, 4× wheel velocity, 4× steering position. Inputs `/cmd_vel`, per-wheel velocity, position and rotation. Kinematics per ros2_controllers mobile-robot kinematics. | Jakub K |
| `robot_state_publisher` | `/tf` | Patryk |

### Perception and localization
| Node | Inputs → Outputs | Owner |
|---|---|---|
| `orb_slam3_node` | `/imu`, `/stereo_image`, `/gps` (optional) → `/visual_odom` | Damian |
| `ekf_node_odometry` | `/visual_odom`, `/wheel_odom`, `/imu` → `/odometry` | |
| `yolo_segmentation_node` | `/image` → `/segmentation_mask` (with IDs) | Nikodem W |
| dynamic/static separator | `/segmentation_mask` → `/dynamic_masks`, `/static_masks` | |
| `nvblox_node` | depth, lidar, `camera_info`, `/dynamic_masks`, `/tf` → `/esdf_map`, `/mesh`, `/static_map`, `/diagnostics` | |

### Planning: Simple Planner
- A state machine: IDLE → PLANNING (global path with OMPL on terrain) →
  EXECUTING (send `cmd_vel`, watch for getting stuck) → COMPLETED → IDLE.
  On FAILED it retries up to N times, then goes back to IDLE.
- OMPL validity checker on the nvblox ESDF, a motion validator against
  terrain obstacles, and a terrain-aware state space.
- Parameters: movement limits, robot size (from the 3D model), collision
  spheres, slope limits and passability.
- Inputs `/goal`, `/manual_command`, `/stop`, `/esdf_map`, `/tf`.
  Outputs `/cmd_vel`, `/planned_path`, `/diagnostics`.

### Control: RL terrain controller
- Inputs: `/cmd_vel` from the planner, `/joint_states`, `/orientation`, a
  local ESDF or 2.5D map, `/dynamic_masks`, `/tf`.
- Local planning: score candidate velocities in a local window for
  collision risk and passability, then pick one. No stored trajectory.
- Terrain-aware control: turn the chosen velocity into joint commands
  (wheels, steering, suspension), share traction across wheels, and adapt
  suspension to the terrain.
- Outputs: `/multiple_joint_command_outputs`, `/diagnostics` (terrain type,
  traction estimate, obstacles, chosen velocity).
- `controller_manager` with the hardware interfaces provides `/wheel_odom`.

### Later features (the "Extended Read Map")
1. Point-to-point navigation through two or more user-placed points, with an optional loop.
2. Exploration of unknown terrain driven by the ESDF, with a GUI and an area constraint.
3. Following the closest person, started from a GUI button.
4. Semantic memory: remember objects and rooms on the 3D mesh, then do
   language-specified tasks (with an LLM) and manipulation (with RL).

## Alternatives

None recorded. One option already in the repo is to keep Nav2 and replace
only the controller plugin. It has not been evaluated against the
OMPL-plus-RL design.

## Open questions
- Q1: is Nomad-R the target of this pipeline, with Nomad-T kept only as it is?
- Q12: should the Simple Planner be a Nav2 planner plugin or a separate node?
