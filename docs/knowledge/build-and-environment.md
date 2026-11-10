# Build and environment (`repo_init.sh`, `repo_build.sh`, `compile.sh`)

How the workspace is set up and built on a development PC and in Docker.
The target platform is Ubuntu 24.04 with ROS 2 Jazzy.

## Repository layout

| Folder | What it holds |
|---|---|
| `nomad_core/` | Robot logic: bringup, navigation, sensor fusion, nvblox, teleop driver, mission packages, MoveIt config. |
| `nomad_hardware/` | `ros2_control` hardware plugins (CAN sensors, VESC), sensor launch and config (lidar, cameras, GPS). |
| `nomad_simulation/` | URDF/USD robot models for RViz, Gazebo (Nomad-T) and Isaac Sim (Nomad-R). |
| `dependencies/` | Source submodules built outside colcon: Livox-SDK2, Pangolin, OpenCV, ORB-SLAM3. |
| `generated/` | Output of the ModuCard generator (ROS message packages for MC modules). |
| `services/`, `scripts/` | systemd units and helper shell scripts for the rover. See [network-and-deployment.md](network-and-deployment.md). |
| `docker/` | Multi-arch Docker image (amd64 and arm64). |
| `configs/` | u-blox ZED-F9P receiver config dump (`simple_ublox_zed_f9p_Rover_5Hz.txt`). |

Submodules are listed in `.gitmodules`. Several of them are the team's own
repositories: `moducard_ros`, `ros_can_device`, `ros_vesc_hardware`,
`ros_ari_shared_types`, `can_constants`, `Pilot_6_axis`,
`Autonomous-Explorer-and-Mapper-ros2-nav2`, `ORB-SLAM3_Ubuntu24.04` and the Livox forks.

## Prerequisites

- ROS 2 Jazzy in `/opt/ros/jazzy`.
- Isaac ROS (bare metal, Ubuntu 24.04), installed per
  <https://nvidia-isaac-ros.github.io/getting_started/index.html>.
  `repo_init.sh` exits with `INSTALL ISAAC ROS !` if `$ISAAC_ROS_WS` is not a
  directory. When it exists, the scripts run `isaac-ros activate`.

## Build steps

1. `./repo_init.sh`: updates submodules, creates `.venv`, installs
   `requirements.txt` (pinned versions, e.g. `cantools==40.2.2`,
   `python-can==4.5.0`, `numpy==2.3.0`), installs pre-commit hooks.
2. `./repo_build.sh`: runs the CAN code generators
   (`ros_can_device/can_constants/generate-files.sh`,
   `can_device/can_constants/can_messages/generate-files.sh`), runs
   `rosdep install` for the three source folders, then
   `dependencies/local_build.sh`, then `./compile.sh`.
3. `./compile.sh`: sources `.venv`, ROS, `setup_local_env.sh` and Isaac ROS,
   then runs
   `colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`.

The same steps exist as VS Code tasks in `.vscode/tasks.json`.

## Local dependency install

`dependencies/local_build.sh` installs into `local_install/` (git-ignored)
rather than system-wide. Only missing apt packages are installed. It builds:

| Dependency | Version | Build type |
|---|---|---|
| Livox-SDK2 | submodule HEAD | Release |
| Pangolin | `v0.9.2` | Release |
| OpenCV | `4.12.0` | Release, Ninja |
| ORB-SLAM3 (Ubuntu 24.04 fork) | `main` | its own `build.sh`, Sophus installed to `local_install/` |

It then writes `setup_local_env.sh` (git-ignored). That script prepends
`local_install/` to `CMAKE_PREFIX_PATH`, `LD_LIBRARY_PATH`, `PKG_CONFIG_PATH`
and `PATH`. Source it before every build or run.

`dependencies/system_build.sh` is the older system-wide variant (uses `sudo
make install`). It also installs `gpsd`, `rtklib` and adds the user to
`dialout` and `tty`. Local installs replaced it on 2026-04-06 (commit `1d64efa`).

## Docker

- `docker/build.sh` builds `nomad:latest` (amd64). `-a` builds `nomad:arm64`,
  `-c` saves the image compressed with zstd (`nomad.tar.zst`; needs `pv` and `zstd`).
- The image is based on `ros:jazzy`. The container runs an infinite loop by
  default. You exec into it to run commands.
- Compose files: `docker-compose.yml` is the base, plus `docker-compose.x86.yml`
  or `docker-compose.arm64.yml` (Raspberry Pi 5 edge nodes).
- `docker/README.md` estimates the first build at about 20 to 30 minutes on an x86 PC. Not measured by us.

## Code style

- `.pre-commit-config.yaml`: `trailing-whitespace`, `check-yaml`, and
  `clang-format` v22.1.4 for C/C++ using `.clang-format`.

## Known limitations (as of 2026-11-10)
- `clenup.sh` does not work. The first line is `rm -rf ` with nothing after
  it, and the next line ends with `\ ` (backslash, space), so the line
  continuation breaks. Running it deletes nothing.
- `compile.sh` always builds `Debug`. `RelWithDebInfo` is only in a comment.
- `simple_driver/simple_driver.py` defaults `can_db` to an absolute path under
  `/home/lemonx/...`. `launch/simple_driver.py` overrides it with a relative
  path to `can_messages.dbc`.
