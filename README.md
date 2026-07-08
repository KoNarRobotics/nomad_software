# Nomad

## HELPER
- Create python package
```bash
ros2 pkg create --license MIT --build-type ament_python NAME
```
- Create Cpp package
```bash
ros2 pkg create --license MIT --build-type ament_cmake NAME
```

## Building the software

### Install Nvidia ROS package nad repos

1. Go to: https://nvidia-isaac-ros.github.io/getting_started/index.html
2. Install Isaac ros for **Ubuntu 24.04 Noble**
3. With **Bare Metal** config
3. Thats it.

### Local Build
1. Clone the repository:
```bash
git clone --recursive git@github.com:KoNarRobotics/nomad_software.git
```
2. Run the initialization script to set up the environment for development:
```bash
./repo_init.sh
```
3. To build repo from scratch with dependencies run:
```bash
./repo_build.sh
```
4. To build only the code without dependencies (you at lest have to build dependencie once) run (you can also just use this as a normal build command):
```bash
./compile.sh
```

### VSCODE
* you can also use VSCODE Tasks: to run all this commands

### Docker Build
* Build the docker image:
```bash
docker/build.sh
```
#### Args
You can also specify a args to build for x86, ARM or if you already want to save the image to file so it can be transferred to the rover:
* For x86:
```bash
docker/build.sh
```
* To save the image to file:
```bash
docker/build.sh -c
```
* For ARM:
```bash
docker/build.sh -a
```

## Services

### Installing services

To install the services on the rover, run the following command:

```bash
./scripts/./make_service.sh
```

### Services running on the rover:

- nomad_base.service (diffbot)
- nomad_nav.service (navigation)

### You can see status using:

- To see their status use:

```bash
sudo systemctl status nomad_nav.service
```

- To view log messages live use:

```bash
sudo journalctl -u nomad_nav.service -f
```

## IP configuration:

### IP config local:

- GATE: `192.168.20.1`
- DHCP: `192.168.20.1`
- MASK: `255.255.255.0`
- DHCP range: `192.168.20.100 - 250`
- STATIC addresses allowed : `192.168.20.2 - 99`

### IP config VPN:

- GATE: `10.10.0.1`
- DHCP: `10.10.0.1`
- MASK: `255.255.0.0`
- DHCP: `None`
- STATIC addresses allowed : `10.10.0.2 - 10.10.255.254`

### IP LIST:

local IP [, vpn IP]

- Router: `192.168.20.1`
- OBC: `192.168.20.2`, `10.10.0.6`
- KonARM (sdrac): `192.168.20.10`
- Livox 360: `192.168.20.20`
- PC Rtx: `192.168.20.30`

## Software Diagram

DrawIO amazing software diagram presenting how its all connected:
[Diagram](https://drive.google.com/file/d/13Ft7TU0lAM-w3s6LpOIg76KfH13r2Nhr/view?usp=sharing)

## Connecting to OBC

#### SSH - Basic Steps

1. Ask the admin to add your ssh key to the Nomad.

#### SSH - With Nomad WIFI

1. Connect to the Nomad network using the following credentials:

```bash
SSID: NomadT
Password: NomadToliwka
```

2. Connect over ssh using the following command:

```bash
ssh ubuntu@192.168.20.2
```

#### SSH - With VPN (Remote access)

1. Request config file to VPN from the admin.
2. Connect to the VPN network using the following command:

```bash
scripts/./vpn_connect.sh  <path to your vpn config>
```

3. Connect over ssh using the following command:

```bash
ssh ubuntu@10.10.0.6
```

### Connecting to OBC via front panel USB

1. Connect the USB cable to the front panel and OBC.

2. Open the terminal and find which port its connected to. Usually it is connected to /dev/ttyUSB0. To find the port use the following command:

```bash
ls /dev/ttyUSB*
```

3. Connect to the console using the following command [to exit the console press `Ctrl + a` then `Ctrl + x`]:

```bash
picocom -b 1500000  /dev/ttyUSB0
```

if you don't have picocom installed you can install it with the following command:

```bash
sudo apt install picocom
```


### Streaming video from OBC camera

Camera PC
```bash
ffmpeg -f v4l2 -pixel_format mjpeg -framerate 30 -s 1280x720 -i /dev/video0 -preset ultrafast -tune zero_latency -b:v 100M -maxrate 120M -bufsize 600M -f mpegts -omit_video_pes_length 1 udp://10.10.0.8:2000
```

Receiver
```bash
ffplay -fflags nobuffer -flags low_delay -probesize 32 -analyzeduration 1 -strict experimental -framedrop -f mpegts -vf setpts=0 udp://10.10.0.6:2000
```

### Issues:

if you have a problem with interface disconnecting or not being visible.
And your log looks something like this:

```bash
[  107.000000] usb 3-4.2: ch34x converter now attached to ttyUSB0
[  107.000000] usb 3-4.2: usbfs: interface 0 claimed by ch34x while 'brltty' sets config #1
[  107.000000] ch34x ttyUSB0: ch34x converter now disconnected from ttyUSB0
[  107.000000] ch34x 3-4.2:1.0: device disconnected
```

You can fix it by removing brltty package with the following command:

```bash
sudo apt remove brltty
```

[more info about the problem](https://github.com/juliagoda/CH341SER/issues/18)

## Navigation base on

https://docs.nav2.org/setup_guides/index.html#table-of-contents

## Sensor Fusion based on

https://github.com/methylDragon/ros-sensor-fusion-tutorial/blob/master/01%20-%20ROS%20and%20Sensor%20Fusion%20Tutorial.md

### Real time problem scheduler

When you start ros and get error that scheduling is not configured.
[Then check this out ](https://control.ros.org/master/doc/ros2_control/controller_manager/doc/userdoc.html).



# runing with irium camera

```bash
export GSCAM_CONFIG='v4l2src device=/dev/video0 io-mode=2 ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! videoconvert ! video/x-raw,format=RGB'
```
```bash
ros2 run gscam gscam_node --ros-args -p camera_name:=iriun_cam -p frame_id:=iriun_cam_optical -p image_encoding:=rgb8
```

# Testing MC modules
- To generate plugins from yaml config file:
```bash
python3 nomad_core/moducard_ros/moducard_generators/mc_generator.py -r --force nomad_core/mc_drivers/CONFIG_YAML_NAME.yaml
```
- Remember to build after generating plugins:
```bash
./compile.sh
```
- To run main driver module (it won't run unless there is at least one plugin available):
```bash
ros2 run mc_plugin_driver mc_plugin_driver_node --ros-args -p config_file:="nomad_core/nomad_bringup/bringup/config/mc_config.yaml"
```


# RUNNING SIMULATION
1. Setup environment
```bash
source setup_local_env.sh
source /opt/ros/jazzy/setup.bash
source install/setup.bash
```

2. Run remote controller in one terminal
```bash
ros2 run remote_6d remote_6d_node
```

3. Run Nomad-R driver node
```bash
ros2 run simple_driver diff_drive_fix
```

4. Open IsaacSim, load nomad-r model ane a  world model then start the simulation

# ROAD MAP

## Sensor Pipeline

### depth_camera_node
- **Outputs:**
  - `/stereo_image`
  - `/depth_image`
  - `/rgb_image`

### lidar_driver_node
- **Outputs:**
  - `/point_cloud2`

### imu_sensor_node
- **Outputs:**
  - `/imu`

### suspension_driver (jakub K)
Kinematyka Robota
https://control.ros.org/humble/doc/ros2_controllers/doc/mobile_robot_kinematics.html
- **Outputs**
  - `/wheel_odom`
  - /sterowanie kół 4x
  - /sterowanie skrętem kół  pozycja 4x
- **Inputs**
  - /cmd_vel
  - /wheel_velocity x4
  - /wheel_pos x4
  - /wheel_rotation_pos x4

### robot_state_publisher (Patryk)
- **Outputs:**
  - `/tf`

## Perception & Localization

### orb_slam3_node (Damian)
- **Inputs:**
  - `/imu`
  - `/stereo_image`
  - /gps (if available)
- **Outputs:**
  - `/visual_odom`

### ekf_node_odometry
- **Inputs:**
  - `/visual_odom`
  - `/wheel_odom`
  - `/imu`
- **Outputs:**
  - `/odometry`

### yolo_segmentation_node (Nikodem W)
- **Inputs:**
  - `/image`
- **Outputs:**
  - `/segmentation_mask` (with IDs)

### node_separating_dynamic_objects_from_static_ones
- **Inputs:**
  - `/segmentation_mask`
- **Outputs:**
  - `/dynamic_masks`
  - `/static_masks` (for 3D position mapping on the map)

### nvblox_node
- **Inputs:**
  - `/depth_image` (from RealSense/Stereo Cameras)
  - `/lidar_pointcloud`
  - `/camera_info`
  - `/dynamic_masks`
  - `/tf` (position in world frame)
- **Outputs:**
  - `/esdf_map`
  - `/mesh`
  - `/static_map`
  - `/diagnostics`

## Planning & Control

### Simple Planner
**State Machine:**
- **IDLE** - No goal active
- **PLANNING** - Goal received, calculate global path using OMPL on terrain
- **EXECUTING** - Send `cmd_vel` commands, monitor progress (stuck, rides)
- **COMPLETED** → IDLE
- **FAILED** → Retry (up to N attempts) or return to IDLE

**Implementation with OMPL:**
- Custom state validity checker (uses ESDF from nvblox)
- Motion validator (collision checking against terrain obstacles)
- State space definition (terrain-aware path planning)

**Parameters:**
- Movement limits
- Robot size (from 3D model)
- Collision detection spheres
- Terrain properties (slope limits, passability)

**Inputs:**
- `/goal`
- `/manual_command`
- `/stop`
- `/esdf_map` (from nvblox for terrain obstacles)
- `/tf` (current robot pose)

**Outputs:**
- `/cmd_vel`
- `/diagnostics`
- `/planned_path` (global path on terrain)

### RL-based Terrain Controller
**Dual Function: Local Planning Evaluation + Terrain-Aware Control**
- **Inputs:**
  - `/cmd_vel` (from Simple Planner global path)
  - `/joint_states` (current robot configuration)
  - `/orientation` (roll, pitch, yaw)
  - `/world_perception` (ESDF or 2.5D map around robot [live view, local region])
  - `/dynamic_masks` (moving obstacles for local avoidance)
  - `/tf` (current pose in world frame)

- **Local Planning (Internal Evaluation):**
  - Evaluates multiple feasible directions/velocities in local window
  - Assesses collision risk and terrain passability for each candidate motion
  - Selects optimal velocity command based on terrain properties and obstacle avoidance
  - No explicit trajectory stored—only velocity outputs

- **Terrain-Aware Control:**
  - Analyzes local terrain properties (slope, friction, passability)
  - Converts selected velocities into motor commands across joints
  - Distributes power for optimal traction based on terrain type
  - Adapts suspension/drive parameters dynamically

- **Parameters:**
  - Model parameters (trained RL policy for evaluation)
  - Joint limits and smooth constraints
  - Local planning window size
  - Terrain classification thresholds

- **Outputs:**
  - `/multiple_joint_command_outputs` (wheel speeds + suspension + arm commands)
  - `/diagnostics` (terrain type, traction estimate, local obstacles detected, selected velocity)

### controller_manager (Hardware Interfaces)
and all the ros_controllers for the hardware drivers
along with their inputs and outputs
- **Outputs:**
  - `/wheel_odom`



# Extended Read Map
- **POINT-TO-POINT NAVIGATION (A -> B)**
  - allow user to place two or more point for robot to move
  between, add a option to make a robot do a loop or something.

- **Mapping, Explore new Terrain**
  - maybe use some how ESDF to move with a  robot to unknown positions so the mapping algos can automatically map it
  - add some gui to it, mayby allow to make  constraint map
  so robot wont leave the are while mapping or map only certain are

- **Follow a Meat Bag**
  - lock on a closest person to the robot and follow them until stopped.
  - probably some gui button

- **Action time** (we can forget to ever get here)
  - some dynamic algo that basically tells the robot to look around for some TARGET/or perform action like manipulate sth
  but that will most likely require some LLM and a completely new
  algo that will be able to remember on a 3d map object with specific ID Notes even, room naming,  so the robot can recognize a task like:  a collection of some item or go to the "control panel of pump 5 in a machine room 3 and press the red button".
  for now it shouel be just mapping of obejct and rooms so robot can have useful memory or even use the 3d mesh map genrated by the `nvblox` sine its literally Colored 3d mesh if the environment so technically he can also just look for stuff on an already existing world in its memory.     Then there is manipulation of object which basically mens more RL.