#!/bin/bash
BUILD_TYPE=${1:-debug}

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_TESTS=ON
make -j4