source .venv/bin/activate
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install  --packages-skip cv_bridge  image_geometry --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# RelWithDebInfo   Debug