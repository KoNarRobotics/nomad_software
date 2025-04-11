#!/bin/bash


cd "$(dirname "$0")"


sudo ip link set can0 txqueuelen 1000
sudo ip link set dev can0 up type can tq 10 prop-seg 36 phase-seg1 37 phase-seg2 25 sjw 21


source /opt/ros/humble/setup.bash
source ./install/setup.bash

ros2 launch launch/simple_driver.py
