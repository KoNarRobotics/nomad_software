#!/bin/bash

source /opt/ros/jazzy/setup.bash
source setup_local_env.sh

python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements.txt

nomad_hardware/ros_can_device/can_constants/generate-files.sh -v

rosdep install -i --from-path nomad_hardware nomad_simulation nomad_core --rosdistro jazzy -y

./compile.sh

