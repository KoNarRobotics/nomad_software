#!/bin/bash

# Local installation build script
# Installs all dependencies to LOCAL_INSTALL directory instead of system-wide
# Requires CMAKE_PREFIX_PATH to be set when building ROS packages

set -e  # Exit on error

cd "$(dirname "$0")"

# Set local install prefix
LOCAL_INSTALL="$(cd .. && pwd)/local_install"
mkdir -p "$LOCAL_INSTALL"

echo "Installing dependencies to: $LOCAL_INSTALL"

# Function to install only missing packages
install_missing_packages() {
    local missing_packages=()

    # Check which packages are not installed
    for package in "$@"; do
        if ! dpkg-query -W -f='${Status}' "$package" 2>/dev/null | grep -q "ok installed"; then
            missing_packages+=("$package")
        fi
    done

    # If there are packages to install, do it all at once (single sudo prompt)
    if [ ${#missing_packages[@]} -gt 0 ]; then
        echo "Installing missing packages: ${missing_packages[*]}"
        sudo apt update
        sudo apt install -y "${missing_packages[@]}"
    else
        echo "All packages already installed."
    fi
}

## Build and install Livox-SDK2

echo ""
echo "========== Building Livox-SDK2 =========="
cd Livox-SDK2
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$LOCAL_INSTALL" \
  -G "Unix Makefiles"
make -C build -j$(nproc) || exit 1
make -C build install || exit 1
echo "Livox-SDK2 built and installed successfully."
cd ..


## ORB SLAM 3 Dependencies (system packages needed for build)
echo ""
echo "========== Installing system build dependencies =========="
install_missing_packages \
  libeigen3-dev \
  cmake \
  build-essential \
  libglew-dev \
  libboost-all-dev \
  libeigen3-dev \
  libpython3-dev \
  python3-numpy \
  ffmpeg \
  libavcodec-dev \
  libavutil-dev \
  libavformat-dev \
  libswscale-dev \
  libraw1394-dev \
  libjpeg-dev \
  libpng-dev \
  libtiff-dev \
  libx11-dev \
  libxrandr-dev \
  libxi-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libepoxy-dev


# ORB SLAM 3: Build Pangolin locally
echo ""
echo "========== Building Pangolin =========="
cd Pangolin
git checkout v0.9.2
mkdir -p build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$LOCAL_INSTALL"
make -j$(nproc)
make install
cd ../..

# Build OpenCV 4.12.0 from source locally (using submodule)
echo ""
echo "========== Building OpenCV 4.12.0 =========="
cd opencv
git checkout 4.12.0
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$LOCAL_INSTALL" \
  -G Ninja
cmake --build build --config Release
ninja -C build install
cd ..

# Build and install ORB SLAM 3 locally
echo ""
echo "========== Building ORB-SLAM3 =========="
cd ORB-SLAM3_Ubuntu24.04
chmod +x build.sh
# Set CMake paths so ORB-SLAM3 can find locally-built OpenCV and Pangolin
export CMAKE_PREFIX_PATH="$LOCAL_INSTALL:$CMAKE_PREFIX_PATH"
export LD_LIBRARY_PATH="$LOCAL_INSTALL/lib:$LOCAL_INSTALL/lib64:$LD_LIBRARY_PATH"
git checkout main
./build.sh
cd Thirdparty/Sophus/build
make install DESTDIR="$LOCAL_INSTALL" || true
cd ../../../..


echo ""
echo "========== Generating setup script =========="
# Generate setup script for environment variables
cat > ../setup_local_env.sh << 'EOF'
#!/bin/bash
# Source this script before building ROS packages
# Usage: source setup_local_env.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOCAL_INSTALL="$SCRIPT_DIR/local_install"

export CMAKE_PREFIX_PATH="$LOCAL_INSTALL:$CMAKE_PREFIX_PATH"
export LD_LIBRARY_PATH="$LOCAL_INSTALL/lib:$LOCAL_INSTALL/lib64:$LD_LIBRARY_PATH"
export PKG_CONFIG_PATH="$LOCAL_INSTALL/lib/pkgconfig:$LOCAL_INSTALL/lib64/pkgconfig:$PKG_CONFIG_PATH"
export PATH="$LOCAL_INSTALL/bin:$PATH"

echo "Local environment configured:"
echo "  CMAKE_PREFIX_PATH: $LOCAL_INSTALL"
EOF
chmod +x ../setup_local_env.sh

echo ""
echo "========== Installation Complete =========="
echo ""
echo "Before building ROS packages, run:"
echo "  source setup_local_env.sh"
echo ""
echo "Then build with:"
echo "  colcon build"
echo ""
