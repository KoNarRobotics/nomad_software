source .venv/bin/activate
source /opt/ros/jazzy/setup.bash
source setup_local_env.sh

if [ -d "$ISAAC_ROS_WS" ]; then
isaac-ros activate
fi

colcon build --symlink-install  --cmake-args -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# RelWithDebInfo   Debug
#--parallel-workers 0