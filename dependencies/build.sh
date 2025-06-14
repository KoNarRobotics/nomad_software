##!/bin/bash


cd "$(dirname "$0")"

cd Livox-SDK2
git submodule update --init --recursive
cmake -B build -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" 

make -C build -j$(nproc) || exit 1

sudo make -C build install || exit 1

echo "Livox-SDK2 built and installed successfully."
cd ..


