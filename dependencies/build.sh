##!/bin/bash


cd "$(dirname "$0")"

git submodule update --init --recursive


## Build and install Livox-SDK2

cd Livox-SDK2
cmake -B build -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" 
make -C build -j$(nproc) || exit 1
sudo make -C build install || exit 1
echo "Livox-SDK2 built and installed successfully."
cd ..


## ORB SLAM 3 Dependency
sudo apt update
sudo apt install -y libeigen3-dev
sudo apt install -y \
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
  libdc1394-22-dev \
  libraw1394-dev \
  libjpeg-dev \
  libpng-dev \
  libtiff-dev \
  libx11-dev \
  libxrandr-dev \
  libxi-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev


# ORB Pangolin Dependency
echo "Building Pangolin..."
cd Pangolin
git checkout v0.9.2
mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo make install
sudo ldconfig
cd ../..

# build open CV 4.12.0 from source
# mkdir -p opencv-4.12.0
# wget -O opencv.zip https://github.com/opencv/opencv/archive/4.12.0.zip
# unzip opencv.zip
# echo "Building OpenCV"
# cd opencv
# git checkout 4.12.0
# cmake -B build -D CMAKE_BUILD_TYPE=Release -G Ninja
# cmake --build build --config Release 
# cd build
# sudo ninja install
# sudo ldconfig
# cd ../..

# Build and install ORB SLAM 3
cd ORB-SLAM3_Ubuntu24.04
chmod +x build.sh
./build.sh
cd Thirdparty/Sophus/build
sudo make install
sudo ldconfig
cd ../../../..


## Other dependencies can be added here


# Build and install gpsd fro gps rtk

sudo apt update
sudo apt install -y scons libgtk-3-dev
sudo apt install gpsd gpsd-clients rtklib
sudo adduser $USER dialout
sudo adduser $USER tty
sudo usermod -aG dialout "$USER"
sudo usermod -aG tty "$USER"


