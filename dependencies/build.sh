##!/bin/bash


cd "$(dirname "$0")"
git submodule update --init --recursive


## Build and install Livox-SDK2

cd Livox-SDK2
cmake -B build -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" 

make -C build -j$(nproc) || exit 1

sudo make -C build install || exit 1

echo "Livox-SDK2 built and installed successfully."


## Build and install gpsd fro gps rtk

sudo apt update
sudo apt install -y scons libgtk-3-dev
sudo apt install gpsd gpsd-clients rtklib
sudo adduser $USER dialout
sudo adduser $USER tty
sudo newgrp dialout
sudo newgrp tty



