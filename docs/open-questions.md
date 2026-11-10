# Open questions

When a question is answered, write the answer next to it (or move it to
`decisions.md`) and update the linked proposal.

Q1 to Q12 came from reading the code on 2026-11-10. Several of them are
probable bugs that need someone to confirm them on the rover.

## Open

| # | Question | Default assumed in proposals | Affects |
|---|---|---|---|
| Q1 | Is Nomad-R the target platform for new autonomy work, with Nomad-T kept only in its current state? | Yes. New work targets Nomad-R. | 0001 |
| Q2 | `diffbot.launch.py` still starts `diff_drive_fix`, which is now the Isaac Sim joystick driver, so nothing publishes `odom_fix` for the EKF. Should the EKF read `/nomad_diffdrive_base_controller/odom` directly, and should the node be removed from bringup? | Unintended. EKF currently fuses only the IMU. | [rover-bringup](knowledge/rover-bringup.md#known-limitations-as-of-2026-11-10) |
| Q3 | `nomad_bringup`'s `ros_control_vesc.xacro` uses the old VESC parameter format (`gear_ratio`, no `prefix`, no `vesc_base_ids`). Should it be replaced by the submodule's own xacro? | Bringup fails on current submodule. Not tested. | [rover-bringup](knowledge/rover-bringup.md#known-limitations-as-of-2026-11-10) |
| Q4 | The NTRIP user name and password are committed in plain text in `gps_rtk/config/ntrip_config.yaml`. Should they move to a git-ignored file or an environment variable? | Left as is. | [gps-rtk](knowledge/gps-rtk.md) |
| Q5 | `docs/gps-rtk-ASG-EUPOS.pdf` is a printed account-activation email committed to git. Should it move to the git-ignored `inbox/`? | Stays in git. Facts extracted to gps-rtk.md. | [sources](sources.md) |
| Q6 | Is `wheel_separation_multiplier: 3.5` a calibrated value or a workaround for skid-steer slip? | Hand-tuned workaround that depends on the surface. | [rover-bringup](knowledge/rover-bringup.md#controller-manager-bringupconfignomad_controllersyaml) |
| Q7 | What is the real Nomad-T footprint? The local costmap uses 1.30 × 0.74 m, the global 0.70 × 0.60 m, and the laser box filter 0.905 × 0.70 m. | Unknown. Configs left as they are. | [localization-and-navigation](knowledge/localization-and-navigation.md) |
| Q8 | Is the `imu_link` mounting (pitch π/2) correct for the physical IMU? | Assumed correct. | [conventions](knowledge/conventions.md) |
| Q9 | Should the large USD assets (`nomad_r.usd` ≈76 MB, `nomad_complete/`) use Git LFS? | Plain git. | [simulation-isaac](knowledge/simulation-isaac.md) |
| Q10 | Where is `can_control.service` defined? Both ROS services require it, but the repo has only `scripts/can_init.service`, and its `ExecStart` path is wrong. | It exists only on the rover. | [network-and-deployment](knowledge/network-and-deployment.md#known-limitations-as-of-2026-11-10) |
| Q11 | `zed_f9p.yaml` sets `tmode3: 1` (Survey-In, a base-station mode) on the rover's receiver. Is that intended? | Unknown. | [gps-rtk](knowledge/gps-rtk.md) |
| Q12 | Should the Simple Planner of proposal 0001 be a Nav2 planner/controller plugin or a standalone node? | Standalone node, as written in the README. | 0001 |

## Answered

| # | Question | Answer |
|---|---|---|
