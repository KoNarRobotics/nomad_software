# Decisions

Newest first. One entry per decision: what, why, and where it is described.

The entries dated before 2026-11-10 were reconstructed from commit history
when the knowledge base was set up. Where no reason was written down, the
entry says so. Add the reason when someone remembers it.

## 2026-07-08 — Isaac ROS is a required dependency

- **The workspace requires Isaac ROS (bare metal, Ubuntu 24.04).**
  `repo_init.sh` stops if `$ISAAC_ROS_WS` is missing (commits `95a9885`,
  `4e133bb`). Why: nvblox comes from Isaac ROS.
  → [build-and-environment.md](knowledge/build-and-environment.md#prerequisites),
  [perception-nvblox.md](knowledge/perception-nvblox.md)

## 2026-05-13 — Nomad-R is simulated in Isaac Sim

- **Nomad-R simulation uses NVIDIA Isaac Sim (USD stage), not Gazebo**
  (commit `7cecfaf`). Why: not recorded. Probably the RTX lidar and camera
  sensors and Isaac ROS integration (unconfirmed).
  → [simulation-isaac.md](knowledge/simulation-isaac.md)

## 2026-04-06 — Dependencies are installed locally

- **Livox-SDK2, Pangolin, OpenCV 4.12.0 and ORB-SLAM3 are built into
  `local_install/` instead of system-wide**, and a Docker image was added
  (commit `1d64efa`). Why: not recorded. Likely reasons are no `sudo make
  install` on dev machines and a reproducible Docker build (unconfirmed).
  → [build-and-environment.md](knowledge/build-and-environment.md#local-dependency-install)

## 2025-11-21 — VESCs run in FOC mode

- **The drive VESCs were switched from BLDC to FOC/HAL** (commit `5f5677c`).
  Why: not recorded.
  → [can-and-hardware-interfaces.md](knowledge/can-and-hardware-interfaces.md#vesc-motor-controllers)

## 2025-10-23 — RTK corrections from ASG-EUPOS

- **GPS RTK uses a u-blox ZED-F9P with NTRIP corrections from ASG-EUPOS**
  (commit `ae9a516`). Why: ASG-EUPOS is the Polish national network and has
  been free since 2022-10-02.
  → [gps-rtk.md](knowledge/gps-rtk.md)

## 2025-06-23 — Stamped velocity commands

- **`/cmd_vel` is `geometry_msgs/TwistStamped` across Nav2, teleop and the
  drive controller** (commit `41b1c4f`). Why: not recorded. ROS 2 Jazzy's
  `diff_drive_controller` and Nav2 default to stamped twists.
  → [teleoperation.md](knowledge/teleoperation.md), [localization-and-navigation.md](knowledge/localization-and-navigation.md#nav2-nomad_navigationlaunchnav2launchpy-confignav2yaml)
