source /opt/ros/humble/setup.bash

git submodule update --init --recursive
nomad_hardware/can_device/can_constants/generate-files.sh -v

