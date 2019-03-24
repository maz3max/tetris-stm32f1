#!/bin/bash
apt-get install -y git libusb-1.0.0-dev
mkdir -p stlink/build_release
cd stlink
git clone https://github.com/texane/stlink.git src
cd build_release
cmake ../src -DCMAKE_BUILD_TYPE=Release
make -j4 install
ldconfig
cd ../..
