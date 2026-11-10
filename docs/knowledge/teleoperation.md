# Teleoperation (`nomad_core/simple_driver`, `nomad_core/Pilot_6_axis`)

Manual driving uses the team's Wi-Fi "Pilot 6-axis" remote. The remote has
two 3-DOF joysticks and 10 buttons and sends UDP packets that become
`sensor_msgs/Joy` on `/joy`. A switch node sends the joystick either to the
rover or to the KonARM arm.

## Remote receiver nodes (`Pilot_6_axis` submodule)

- `remote_6d` (C++). The submodule README recommends it. It applies a
  deadband and sends a zero frame when the remote disconnects. Run it with
  `ros2 run remote_6d remote_6d_node`. The simulation steps use it.
- `pilot_6axis` (Python, executable `rc_node`, param
  `pilot_six_axis_joy_topic: /joy`). The README marks it deprecated, but
  `nomad_bringup/launch/rc.launch.py` still uses it on the rover.
- `keyboard_rc.py`: a keyboard stand-in for testing without the remote.

## Topic chain on the rover (`rc.launch.py`)

```
remote → rc_node → /joy → rc_switch ─┬─ /nomad/joy → rc_driver → /cmd_vel (TwistStamped) → diff drive controller
                                     └─ /sdrac/joy → KonARM
```

### `rc_switch` (`simple_driver/rc_switch.py`)
- Button 6 (rising edge) sends the joystick to KonARM (`/sdrac/joy`). Button 7 sends it to Nomad (`/nomad/joy`). The default is Nomad.
- On every switch it first publishes an all-zero `Joy` to the old target so that target stops.
- A 1 s timeout zero-frame exists in the code, but the timer that would call it is commented out.

### `rc_driver` (`simple_driver/rc_driver.py`)
- Publishes `geometry_msgs/TwistStamped` on `/cmd_vel`, frame `base_link`. Nav2
  was switched to stamped `cmd_vel` on 2025-06-23.
- `linear.x = axes[0] · 3.5 · k` m/s and `angular.z = −axes[1] · 1.428571429 · k` rad/s.
- `k` is the parameter `kaganiec_predkosci` (Polish for "speed muzzle").
  The node default is 0.1. `rc.launch.py` sets 0.7.
- Button 8 held: "TURBO", `k = 1`.
- Button 5: cancels the current Nav2 `navigate_to_pose` goal.

## Other drivers in `simple_driver`

| Executable | File | What it does |
|---|---|---|
| `sd_node` | `simple_driver.py` | Old direct-CAN driver that sends VESC duty cycles with python-can and cantools. Params: `kaganiec_predkosci` (0.2), `turn_radius` (0.8), `can_db`. Launched by `launch/simple_driver.py` together with `rc_node` and `rc_switch`. |
| `diff_drive_fix` | `diff_drive_fix.py` | Isaac Sim Nomad-R joystick driver. See [simulation-isaac.md](simulation-isaac.md#driving-the-simulated-rover). |
| (none) | `nav_css.py` | Logs `/ublox_gps_node/fix` to a hard-coded CSV path. Not registered as an entry point. |
| (none) | `can_device.py` | python-can helper. |
