#!/bin/sh -x

clang-format -style=file -i *.cpp
clang-format -style=file -i *.h
cmake-format -i CMakeLists.txt doc/CMakeLists.txt tests/CMakeLists.txt
