#!/bin/bash

set -e

rm generated_main.cpp > /dev/null

./cpp-rinher-compiler $1


clang-format -i generated_main.cpp

/usr/bin/c++ -O3 -DNDEBUG generated_main.cpp -o cpp-rinher-runner  -ljsoncpp -flto

./cpp-rinher-runner
