# Project log

Chronological record of meetings and work sessions, newest first.
Each entry: what happened, decisions, and next steps. Details live in the
linked pages; this file is the short version.

---

## 2026-11-10 — Knowledge base set up

**Done**
- Created the knowledge base in [`docs/`](README.md) and the
  [`inbox/`](../inbox/README.md) for source documents (git-ignored).
- Wrote 12 knowledge pages from the code as of commit `d0e4e73`
  (2026-07-08) plus the uncommitted Isaac Sim assets. See the
  [index](README.md#pages).
- Turned the README road map into [proposal 0001](proposals/0001-autonomy-pipeline.md) (draft).
- Reconstructed past decisions from commit history → [decisions.md](decisions.md).

**Project goal (as inferred, to be confirmed)**
- Nomad is KoNaR Robotics' field rover. The software is ROS 2 Jazzy on
  Ubuntu 24.04. It covers teleoperation, 2D SLAM with Nav2 on the current
  Nomad-T, mission tasks (radiation mapping, sign detection, arm), and a
  planned terrain-aware autonomy stack for Nomad-R (nvblox ESDF, OMPL planner,
  RL controller).

**Found** (from reading the code, none verified on the rover)
- `diffbot.launch.py` runs `diff_drive_fix`, which is now an Isaac Sim
  driver, so the EKF gets no wheel odometry → Q2.
- `nomad_bringup`'s VESC xacro does not match the `ros_vesc_hardware`
  submodule's parameters → Q3.
- NTRIP credentials are in git → Q4. `clenup.sh` deletes nothing.
- The services depend on a `can_control.service` that is not in the repo → Q10.

**Next**
- Confirm the project goal and Q1 with the team.
- Check Q2 and Q3 on the rover (`ros2 topic hz /nomad_diffdrive_base_controller/odom_fix`, start the base service).
- Decide on Q4 and Q5 (secrets and the email PDF in git).
