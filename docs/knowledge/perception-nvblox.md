# 3D mapping with nvblox (`nomad_core/nomad_nvblox`)

nvblox (Isaac ROS) builds a GPU TSDF/ESDF map and a coloured mesh from
depth images, and optionally from lidar. In the road map it supplies the
ESDF that the planner and the RL controller use
([proposal 0001](../proposals/0001-autonomy-pipeline.md)). Added on
2026-07-08 (commit `95a9885`). It needs Isaac ROS, see
[build-and-environment.md](build-and-environment.md#prerequisites).

## Launch files

- `launch/simple_nvblox.launch.py`: uses the Isaac ROS launch-argument
  helpers. Arguments: `rosbag`, `rosbag_args`, `log_level`, `num_cameras`
  (default 1), `mode` (default `NvbloxMode.static`), `use_yaml_config`.
  When a rosbag is given, it sets `use_sim_time: true`.
- `launch/nvblox.launch.py`: input topics are launch arguments. Defaults:
  - depth `/nvblox/depth/image_raw` and `/nvblox/depth/camera_info`
  - colour `/nvblox/camera/image_raw` and `/nvblox/camera/camera_info`
  - mask camera info `/nvblox/mask/camera_info`
  - a lidar point cloud argument
- `launch/pre_nvblox.py`, `launch/realsense_example.launch.py`: helper and an upstream RealSense example.

## Base config (`config/nvblox_base.yaml`)

| Parameter | Value |
|---|---|
| `voxel_size` | 0.05 m |
| `mapping_type` | `static_tsdf` |
| `esdf_mode` | `3d` |
| `use_depth` / `use_color` / `use_lidar` | true / true / false |
| `integrate_depth_rate_hz`, `integrate_lidar_rate_hz` | 40 |
| `integrate_color_rate_hz`, `update_mesh_rate_hz` | 5 |
| `update_esdf_rate_hz` | 10 |
| `map_clearing_radius_m` | 7.0, around `base_link` |
| lidar model (unused) | `lidar_width 1800`, `lidar_height 31`, min range 0.1 m |

Mode-specific overlays: `nvblox_dynamics.yaml`, `nvblox_segmentation.yaml`, `nvblox_detection.yaml`.

## Known limitations (as of 2026-11-10)
- The `lidar_width` and `lidar_height` defaults do not describe the MID-360.
  That does not matter while `use_lidar: false`.
- No rover or simulation launch remaps real camera topics onto the
  `/nvblox/...` defaults yet. Not tested end to end.
