source /opt/ros/humble/setup.bash

git submodule update --init --recursive
nomad_hardware/can_device/can_constants/generate-files.sh -v

rosdep install -i --from-path nomad_hardware --rosdistro humble -y
rosdep install -i --from-path nomad_simulation --rosdistro humble -y
rosdep install -i --from-path nomad_core --rosdistro humble -y



