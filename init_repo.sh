source /opt/ros/jazzy/setup.bash

python3 -m venv .venv
source .venv/bin/activate

python3 -m pip install -r requirements.txt

pre-commit install
pre-commit autoupdate
pre-commit install

git submodule update --init --recursive
./dependencies/./build_local.sh
source setup_local_env.sh

nomad_hardware/ros_can_device/can_constants/generate-files.sh -v

rosdep install -i --from-path nomad_hardware --rosdistro jazzy -y
rosdep install -i --from-path nomad_simulation --rosdistro jazzy -y
rosdep install -i --from-path nomad_core --rosdistro jazzy -y


./compile.sh

