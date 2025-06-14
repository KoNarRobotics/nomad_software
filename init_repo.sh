source /opt/ros/jazzy/setup.bash

python3 -m venv .venv
source .venv/bin/activate


git submodule update --init --recursive
./dependencies/build.sh


nomad_hardware/can_device/can_constants/generate-files.sh -v

rosdep install -i --from-path nomad_hardware --rosdistro jazzy -y
rosdep install -i --from-path nomad_simulation --rosdistro jazzy -y
rosdep install -i --from-path nomad_core --rosdistro jazzy -y



