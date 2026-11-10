# Network and deployment (`services/`, `scripts/`)

How the onboard computer (OBC) is reached and how the software starts on
boot. Connection steps such as SSH, VPN and the serial console are in the
root `README.md`. This page keeps the facts in one place.

## Network

Local network `192.168.20.0/24`. Gateway and DHCP server `192.168.20.1`.
The DHCP range is `.100` to `.250`, and static addresses go in `.2` to `.99`.
The VPN is `10.10.0.0/16` with gateway `10.10.0.1`, no DHCP, and static
addresses from `10.10.0.2` up.

| Device | Local IP | VPN IP |
|---|---|---|
| Router | `192.168.20.1` | |
| OBC | `192.168.20.2` | `10.10.0.6` |
| KonARM (sdrac) | `192.168.20.10` | |
| Livox MID-360 | `192.168.20.20` | |
| PC (RTX) | `192.168.20.30` | |

- SSH: `ssh ubuntu@192.168.20.2` (local) or `ssh ubuntu@10.10.0.6` (VPN).
  An admin has to add your key. The VPN is joined with `scripts/vpn_connect.sh <config>`.
- Serial console through the front-panel USB:
  `picocom -b 1500000 /dev/ttyUSB0`. If the CH34x adapter keeps
  disconnecting, remove `brltty`.
- The Wi-Fi SSID and password are in the root `README.md`.

## systemd services

`scripts/make_service.sh` copies `services/nomad_base.service` and
`services/nomad_nav.service` to `/etc/systemd/system` and enables them.

| Unit | Runs | Depends on |
|---|---|---|
| `nomad_base.service` | `services/start_base.sh` → `ros2 launch nomad_bringup diffbot.launch.py` | `can_control.service` |
| `nomad_nav.service` | `services/start_nav.sh` → `ros2 launch nomad_navigation navigation.launch.py` | `nomad_base.service` |
| `nomad_dumy.service` (not installed) | `/home/ubuntu/it/nomad_software/start.sh` | `can_control.service` |
| `scripts/can_init.service` | `/home/ubuntu/it/nomad_software/start_can.sh` (root, oneshot) | |
| `scripts/ros_rosbag.service` | `/home/ubuntu/scripts/led_status.sh`. The description says "Led status control". | |

- All units run as user `ubuntu`, with the repo at `/home/ubuntu/it/nomad_software`.
- The start scripts source `.venv`, add its `site-packages` to `PYTHONPATH`,
  set `QT_SCREEN_SCALE_FACTORS=1`, then source ROS and `install/setup.bash`.
- Status and logs: `sudo systemctl status nomad_nav.service`, `sudo journalctl -u nomad_nav.service -f`.

## Known limitations (as of 2026-11-10)
- The services require `can_control.service`, but the repo only has
  `can_init.service`, and `make_service.sh` does not install it. The CAN unit
  points to `/home/ubuntu/it/nomad_software/start_can.sh`, while the script is
  at `scripts/start_can.sh`. The rover may have a hand-made
  `can_control.service` that is not in git. Not checked. See Q10.
- `services/start_dummy.sh` launches `launch/simple_driver.py` only after
  `diffbot.launch.py` exits.
- `ros_rosbag.service` does not record rosbags, despite its name.
- `diffbot.launch.py` defaults to `gui:=true`, so the base service tries to
  start RViz on the OBC.
