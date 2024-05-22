#!/usr/bin/env sh
BUILD_DIR=/home/ubuntu/project/build-vm
mkdir $BUILD_DIR -p
cd $BUILD_DIR
cmake ..
make
