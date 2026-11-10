# GPS RTK (`nomad_hardware/gps_rtk`)

The rover uses a u-blox ZED-F9P receiver with RTK corrections from the
Polish ASG-EUPOS network over NTRIP. A separate CAN GPS module also exists
(`gps_hardware`, see [can-and-hardware-interfaces.md](can-and-hardware-interfaces.md#hardware-plugins)).

## Launch: `ros2 launch gps_rtk gps_rtk.launch.py`

Starts two nodes:
- `ublox_gps_node` from the `ublox` submodule (KumarRobotics), with params
  from `config/zed_f9p.yaml`.
- `ntrip_client` (`ntrip_ros.py`), with params from
  `config/ntrip_config.yaml`. `/fix` is remapped to `/ublox_gps_node/fix`
  so the client can send the position (NMEA) to the caster.
  `NTRIP_CLIENT_DEBUG=false`.

## Receiver config (`config/zed_f9p.yaml`)

- `device: /dev/ttyACM1`, `uart1.baudrate: 115200`, `frame_id: gps`.
- `rate: 5.0` Hz, `dynamic_model: automotive`.
- `tmode3: 1` (Survey-In) with `sv_in.min_dur: 300` s and `sv_in.acc_lim: 3.0` m.
  Survey-In is a base-station mode, which is unusual for a moving rover.
  Not verified whether it is intended. See Q11.
- `configs/simple_ublox_zed_f9p_Rover_5Hz.txt` is a u-center config dump
  (MON-VER, CFG-VALGET) for the receiver: firmware HPG 1.51, protocol 27.50,
  constellations GPS, GLONASS, Galileo, BeiDou, plus SBAS and QZSS.

## NTRIP / ASG-EUPOS (`config/ntrip_config.yaml`)

- Caster `system.asgeupos.pl:2101`, mountpoint `RTN4G_VRS_RTCM32`,
  `authenticate: true`, no SSL.
- `rtcm_frame_id: odom`, `rtcm_message_package: rtcm_msgs`.
- Reconnect: up to 10 attempts, 5 s apart. Reconnects after 4 s without RTCM.
- ASG-EUPOS services have been free since 2022-10-02. The account was
  activated on 2025-10-24. The service IP is `91.198.76.2`. Ports: 2101 or
  8080 (NAWGEO RTN), 8082 to 8085 (NAWGEO RTK), 8081 (KODGIS, NAWGIS). The
  NTRIP user name format is `<company>/<user>`. Source: the activation email,
  see [sources.md](../sources.md).

## Known limitations (as of 2026-11-10)
- The NTRIP user name and password are committed in plain text in
  `ntrip_config.yaml`. See Q4.
- The GPS RTK launch is not included in `diffbot.launch.py`. The EKF config
  has a `navsat_fusion.launch.py`, but it is not used by default. See
  [localization-and-navigation.md](localization-and-navigation.md).
- `/dev/ttyACM1` depends on USB enumeration order.
