#! /bin/bash

cmake --build ./build
make --directory=build test
