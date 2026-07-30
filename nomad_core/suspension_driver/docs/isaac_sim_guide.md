# Running `suspension_driver` with NVIDIA Isaac Sim

This guide wires the swerve base in Isaac Sim to the `suspension_driver` node
through an ActionGraph (OmniGraph) using the ROS 2 bridge.

## 1. Prerequisites

- Isaac Sim 4.x with the **ROS 2 Bridge** extension enabled
  (`Window > Extensions > isaacsim.ros2.bridge` / `omni.isaac.ros2_bridge`).
- Isaac Sim launched with the same `ROS_DOMAIN_ID` and RMW implementation as
  your terminal (e.g. `rmw_fastrtps_cpp`).
- A robot USD with an articulation root and 8 joints:
  - 4 continuous **drive** joints (wheel spin), order FL, FR, RL, RR
  - 4 revolute **steering** joints (module rotation), order FL, FR, RL, RR

## 2. ActionGraph (OmniGraph) layout

Create one graph (`Window > Graph Editors > Action Graph`) with the nodes below.

### Common infrastructure

| OmniGraph node | Purpose |
|---|---|
| `On Playback Tick` | Fires every sim frame; drives all ROS nodes |
| `ROS2 Context` | One shared ROS 2 context (set your domain id) |
| `Isaac Read Simulation Time` | Timestamps for publishers |
| `ROS2 Publish Clock` | Publishes `/clock` (run the driver with `use_sim_time:=true`) |

### Command path (driver -> simulation)

1. **`ROS2 Subscribe Twist`** — only if you additionally want a raw teleop
   path; the `suspension_driver` node itself subscribes `/cmd_vel` on the ROS
   side, so nothing in the graph consumes the Twist directly.
2. **Two `ROS2 Subscriber` nodes** (message type `std_msgs/msg/Float64MultiArray`):
   - Topic `/drive_velocity_cmd`
   - Topic `/steering_position_cmd`
3. **Two `Isaac Articulation Controller` nodes**, both pointing at the robot's
   articulation root prim:
   - Drive controller: `jointNames = [fl_drive, fr_drive, rl_drive, rr_drive]`,
     connect the `data` array of the `/drive_velocity_cmd` subscriber to
     **`velocityCommand`**.
   - Steering controller: `jointNames = [fl_steer, fr_steer, rl_steer, rr_steer]`,
     connect the `data` array of the `/steering_position_cmd` subscriber to
     **`positionCommand`**.
   - Wire `On Playback Tick -> execIn` of both controllers, and the
     subscribers' `execOut` into the controllers as well.

The joint name order in `jointNames` **must** match the driver's wheel order
FL, FR, RL, RR — this is the contract for every 4-element array.

### Feedback path (simulation -> driver)

1. **`Isaac Read Articulation` / joint state source** — add a
   `ROS2 Publish Joint State` node (articulation root as target) if you want
   a standard `/joint_states` stream for RViz.
2. **Three `ROS2 Publisher` nodes** (`std_msgs/msg/Float64MultiArray`):
   - `/wheel_velocity` — 4 drive joint velocities [rad/s]
   - `/wheel_pos` — 4 drive joint positions [rad]
   - `/wheel_rotation_pos` — 4 steering joint positions [rad]

   Feed them from the articulation's joint state outputs (use a script node or
   `Isaac Joint State` outputs plus array-index/compose nodes to select and
   order the 4 values FL, FR, RL, RR for each topic).

With this in place the loop is closed: Nav2/teleop publishes `/cmd_vel`, the
driver computes wheel commands, Isaac Sim actuates the articulation, the graph
publishes joint feedback, and the driver integrates `/wheel_odom`.

## 3. Bring-up order

```bash
# Terminal 1 - simulation clock source
# (start Isaac Sim, open the stage, press Play)

# Terminal 2 - the swerve driver
source install/setup.bash
ros2 launch suspension_driver suspension_driver.launch.py use_sim_time:=true

# Terminal 3 - remote 6D input node
ros2 run remote_6d remote_6d_node

# Terminal 4 - keyboard RC teleop feeding the remote
python3 nomad_core/Pilot_6_axis/pilot_6axis/pilot_6axis/keyboard_rc.py -i 127.0.0.1 -p 25000
```

Both commands in terminals 3 and 4 are required to operate the setup: the
`remote_6d_node` bridges the 6-axis remote protocol into ROS, and
`keyboard_rc.py` connects to it on `127.0.0.1:25000` to generate the commands
from the keyboard.

## 4. Verification checklist

- `ros2 topic hz /wheel_velocity` shows the sim feedback rate.
- `ros2 topic echo /drive_velocity_cmd` reacts to keyboard input.
- `ros2 topic echo /wheel_odom` pose changes as the robot drives.
- TF `odom -> base_link` is broadcast (check with `ros2 run tf2_tools view_frames`).
