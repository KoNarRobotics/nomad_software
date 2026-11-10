# Lidar and cameras (`nomad_hardware/camera_360`, `nomad_hardware/stereo_camera_config`)

Exteroceptive sensors on the rover: a Livox MID-360 lidar, three USB
cameras on a "camera 360" mount, and a Luxonis OAK-D (Lite) stereo camera.
Mounting frames are listed in [conventions.md](conventions.md#coordinate-frames).

## Livox MID-360 (`camera_360/launch/lidar.launch.py`)

- Network setup (`camera_360/config/MID360_config.json`): the lidar is at
  `192.168.20.20` and the host (OBC) at `192.168.20.2`. Lidar ports are
  56100 to 56500 and host ports 56101 to 56501. The extrinsic is pitch 10°,
  with no translation.
- The driver is `livox_ros_driver2` (team fork, submodule), built against
  Livox-SDK2 from `dependencies/`. It runs with `xfer_format 0` (PointCloud2
  PointXYZRTL), `multi_topic 0`, `publish_freq 20.0` Hz and `frame_id lidar_link`.

Topic chain:

```
/livox/lidar ─(remap)→ /cloud
   → pointcloud_to_laserscan  (target_frame base_laser, height −0.3…+0.3 m,
                               angle ±π, 1° steps, range 0–40 m)
   → /scan_relay
   → laser_filters scan_to_scan_filter_chain (LaserScanBoxFilter, box_frame base_link,
                               x ±0.4525 m, y ±0.35 m, z 0–2 m removes the robot body)
   → /scan                    used by slam_toolbox and Nav2
/livox/imu → topic_tools throttle 20 Hz → /livox/imu_throttled
```

- The ±0.3 m height band leaves out the ground and the ceiling. The code
  comment says the value was not tuned carefully.
- The box filter's x half-size of 0.4525 m comes from a comment: "0.165 +
  0.03 (wifi antenna) + 0.05 (lidar uncertainty)".

## USB cameras (`camera_360/launch/camera.launch.py`)

Three `usb_cam` nodes, each at 640×480 and 10 fps:

| Camera | Device | frame_id | Calibration |
|---|---|---|---|
| front | `/dev/video2` | `camera_front_link_optical` | `config/camera_info_front.yaml` |
| left | `/dev/video0` | `camera_left_link_optical` | `config/camera_info_left.yaml` |
| right | `/dev/video4` | `camera_right_link_optical` | `config/camera_info_right.yaml` |

`/dev/videoN` numbers depend on USB enumeration order. They are not pinned with udev rules.

## OAK-D stereo (`stereo_camera_config/launch/stereo.launch.py`)

- Uses `depthai_ros_driver_v3` with `config/rgbd.yaml`. The node is `/oak`,
  with `i_pipeline_type: rgbd`, `i_enable_imu: true`, `i_nn_type: none` and
  the stereo topic published.
- Depth filtering options (spatial, temporal, threshold) are in the YAML but commented out.
- The simulation has an OAK-D Lite model in `nomad_simulation/luxonis_oak_d_lite/`.

## Video streaming (manual)

The README has `ffmpeg` (sender, MJPEG 1280×720 at 30 fps, MPEG-TS over UDP)
and `ffplay` (low-latency receiver) commands for streaming over the VPN.

## Known limitations (as of 2026-11-10)
- The OAK-D launch is not included in `diffbot.launch.py`.
