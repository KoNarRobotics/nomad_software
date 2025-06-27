#!/bin/bash

cd "$(dirname "$0")"
dir=$(pwd)
cd "$dir/.."

source .venv/bin/activate

export PYTHONPATH="$dir/.venv/lib/python3.12/site-packages:$PYTHONPATH"
export QT_SCREEN_SCALE_FACTORS=1

source /opt/ros/jazzy/setup.bash
source ./install/setup.bash

ros2 launch nomad_navigation navigation.launch.py
