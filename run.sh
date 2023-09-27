#!/bin/bash


rm -f generated_main.cpp > /dev/null
rm -f cpp-rinher-runner > /dev/null

set -e

./cpp-rinher-compiler $1

#clang-format -i generated_main.cpp

/usr/bin/c++ -O3 -DNDEBUG generated_main.cpp -o cpp-rinher-runner  -ljsoncpp -flto

./cpp-rinher-runner
