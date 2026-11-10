# CAN bus and hardware interfaces (`nomad_hardware/`)

Most rover electronics (motor controllers, IMU, GPS, barometer, Geiger
counter, lights) sit on one CAN bus, `can0`. They are read through
`ros2_control` hardware plugins written by the team.

## CAN interface setup (`scripts/start_can.sh`)

```
ip link set can0 txqueuelen 1000
ip link set dev can0 up type can tq 10 prop-seg 36 phase-seg1 37 phase-seg2 25 sjw 21
```

- Bit timing: 1 + 36 + 37 + 25 = 99 time quanta of 10 ns, so 990 ns per
  bit. That gives about 1.01 Mbit/s with a sample point at 74.7 %. These
  numbers are calculated from the script and not measured.
- `simple_driver/simple_driver.py` opens `can0` with python-can at `bitrate=1000000`.
- The script waits in a loop until `can0` exists.
- `scripts/can_init.service` runs it as root (oneshot). See [network-and-deployment.md](network-and-deployment.md#systemd-services) for a naming mismatch.

## CAN message definitions

- `nomad_hardware/can_device/can_constants` (submodule) and
  `nomad_hardware/ros_can_device/can_constants` hold the message database.
  `generate-files.sh` produces a DBC file
  (`can_messages/output/can_messages.dbc`) and C headers
  (`can_device/can_messages.h`, with `CAN_<NAME>_FRAME_ID` and
  `CAN_<NAME>_IS_EXTENDED` macros). `repo_build.sh` runs both generators.
- `ros_can_device` (submodule, package `can_device`) provides `CanDriver`.
  Plugins call `CanDriver::Make(<interface>)` and register callbacks per frame ID.

## Hardware plugins

| Package | Plugin class | Source size |
|---|---|---|
| `ros_vesc_hardware` (submodule, package `vesc_hardware`) | `vesc_hardware/MCVescHardware` (SystemInterface) | `src/vesc_hardware.cpp`, 493 lines |
| `imu_hardware` | `imu_hardware/MCImuHardware` | 456 lines |
| `gps_hardware` | `gps_hardware/MCGpsHardware` | 348 lines |
| `geiger_hardware` | `geiger_hardware/MCGeigerHardware` | 226 lines |
| `barometer_hardware` | `barometer_hardware/MCBarometerHardware` | 202 lines |
| `gpio_hardware` | `gpio_hardware/MCGpioHardware` | 152 lines |

Each plugin reads the `can_interface` hardware parameter.

Custom broadcasters in `nomad_core`: `barometer_broadcaster/BarometerBroadcaster`
and `geiger_broadcaster/GeigerBroadcaster`. The latter publishes
`geiger_msg/Radiation` (`header`, `float64 siwert`, `float64 cpm`). The IMU
and GPS use the standard `imu_sensor_broadcaster` and `gps_sensor_broadcaster`.

## VESC motor controllers

From `ros_vesc_hardware/src/vesc_hardware.cpp`:

- Each joint has its own `command_interface_type`, `gear_ratio`,
  `polar_pairs`, `current_to_torque` and `vesc_base_id`. These are passed as
  comma-separated lists in the parameters `command_interface_type`,
  `gear_ratios`, `polar_pairs`, `current_to_torque` and `vesc_base_ids`.
  `prefix` is required too.
- CAN IDs: `(CAN_VESC_FLEFT_<MSG>_FRAME_ID & 0xffffff00) | vesc_base_id`.
  The VESC ID is in the low byte.
- Velocity command: `rpm = ω[rad/s] · 60 / 2π · gear_ratio · polar_pairs` (this is ERPM).
- Velocity state: `ω = erpm / 60 · 2π / (gear_ratio · polar_pairs)`.
- Effort state: `current · current_to_torque · gear_ratio`.
- Effort command is sent as a current: `effort / current_to_torque · 1000 / gear_ratio` (mA).
- Position command is in milli-degrees.
- Nomad-T values: `gear_ratio 4.375`, `polar_pairs 7`, `current_to_torque
  0.0665` N·m/A (66.5 mN·m/A for the 6534 BLDC motor). The submodule's own
  xacro uses `vesc_base_ids` 20, 21, 22, 23.
- The VESCs use FOC (switched from BLDC mode on 2025-11-21, commit `5f5677c`).

`nomad_bringup` still uses the old parameter format. See
[rover-bringup.md](rover-bringup.md#known-limitations-as-of-2026-11-10).

## Lights

`scripts/light_control.sh 1|0` sends `cansend can0 00001112#0F` (on) or `#00` (off).

## ModuCard (MC) modules

`nomad_core/moducard_ros` (submodule) is a plugin framework for the team's
CAN modules. It contains `mc_can_driver`, `mc_plugin_base`, `mc_plugin_driver`
and `moducard_generators`.

- A module is described in YAML, for example
  `nomad_core/mc_drivers/mcan_suspension_pos_ecoder.yaml` (the
  `suspension_position_module`, vendor `konar`, 48 V, 0.1 A, with commands
  such as `set_encoder_offset_left` in radians).
- Generate the ROS packages from the YAML:
  `python3 nomad_core/moducard_ros/moducard_generators/mc_generator.py -r --force nomad_core/mc_drivers/<NAME>.yaml`.
  The output goes to `generated/ros/<module>_msgs/` (messages, conversions
  and a plugin `.cpp`). Run `./compile.sh` afterwards.
- Run the driver:
  `ros2 run mc_plugin_driver mc_plugin_driver_node --ros-args -p config_file:="nomad_core/nomad_bringup/bringup/config/mc_config.yaml"`.
  It will not start unless at least one plugin is available.
- `mc_config.yaml`: `can_primary_interface: can0`, `can_secondary_interface:
  can1`, `frame_id: euro_rack`, `states_update_default_rate: 15`,
  `drivers: [suspension_position_module_msgs]`. The module list is empty
  because all entries are commented out.

## Known limitations (as of 2026-11-10)
- `geiger_hardware` and `geiger_broadcaster` are built but disabled in bringup.
- `simple_driver/simple_driver.py` (`sd_node`) drives the VESCs directly with
  duty-cycle CAN frames (`vesc_*_set_duty`) from python-can. This bypasses
  `ros2_control`. It is only launched from `launch/simple_driver.py`.
