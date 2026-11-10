# Knowledge base

Everything we know and decide about Nomad, in plain Markdown.
Code lives in the repo root; this folder holds explanations, plans and decisions.
The root [`README.md`](../README.md) keeps the quick-start commands.

## Layout

| Path | What goes there |
|---|---|
| [knowledge/](knowledge/) | Facts: how the code, tools and hardware work. Should stay true until the code changes. |
| [proposals/](proposals/) | Plans and design options that are not built yet. Each has a status. |
| [log.md](log.md) | Project log: meetings and work sessions, newest first. |
| [decisions.md](decisions.md) | Short log of decisions taken, newest first, with the reason. |
| [open-questions.md](open-questions.md) | Things we still have to decide or find out. |
| [sources.md](sources.md) | Where external information came from (documents processed from `inbox/`, links). |

## Pages

Knowledge
- [Units, frames and naming](knowledge/conventions.md)
- [Build and environment](knowledge/build-and-environment.md)
- [Rover bringup](knowledge/rover-bringup.md)
- [CAN bus and hardware interfaces](knowledge/can-and-hardware-interfaces.md)
- [Teleoperation](knowledge/teleoperation.md)
- [Lidar and cameras](knowledge/sensors-lidar-and-cameras.md)
- [GPS RTK](knowledge/gps-rtk.md)
- [Localization and navigation](knowledge/localization-and-navigation.md)
- [3D mapping with nvblox](knowledge/perception-nvblox.md)
- [Simulation (Isaac Sim)](knowledge/simulation-isaac.md)
- [Network and deployment](knowledge/network-and-deployment.md)
- [Mission and auxiliary packages](knowledge/mission-packages.md)

Proposals
- [0001 Autonomy pipeline for Nomad-R](proposals/0001-autonomy-pipeline.md)

## Conventions

- English only.
- One topic per file. Link between pages with relative links.
- Write facts in `knowledge/`, opinions and plans in `proposals/`.
- When a proposal is accepted, set its status to `accepted`, add a line to
  `decisions.md`, and move the facts it establishes into `knowledge/` once
  the code exists.
- Units are SI; coordinate frames as in [conventions](knowledge/conventions.md#coordinate-frames).
- Dates are YYYY-MM-DD.
- Never write passwords or tokens into `docs/`. Point to the config file instead.
- External material goes through [`inbox/`](../inbox/README.md).
