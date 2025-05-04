#!/bin/bash


cd "$(dirname "$0")"


dir=$(pwd)

source .venv/bin/activate

export PYTHONPATH="$dir/.venv/lib/python3.12/site-packages:$PYTHONPATH"

export QT_SCREEN_SCALE_FACTORS=1
source /opt/ros/jazzy/setup.bash
source ./install/setup.bash

ros2 launch launch/simple_driver.py
