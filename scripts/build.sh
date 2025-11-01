#!/bin/bash
BUILD_TYPE=${1:-debug}

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_TESTS=ON
make -j4
ctest --output-on-failure --verbose

cd ..
gcovr --filter='src/' --print-summary --sort-percentage --gcov-ignore-parse-errors=negative_hits.warn
gcovr --filter='src/' --sonarqube > coverage.xml