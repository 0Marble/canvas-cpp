#! /bin/bash

[ ! -d build ] && mkdir build

cd build
cmake .. -DWITH_GLFW=ON
cmake --build .