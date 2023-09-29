#!/bin/bash

# set -x

rm -f generated_main.cpp > /dev/null
rm -f cpp-rinher-runner > /dev/null
rm -f generated_main.jl > /dev/null

./cpp-rinher-compiler $1 1
if [ $? -eq 0 ]; then

    # clang-format -i generated_main.cpp

    /usr/bin/c++ -std=c++17 -O3 -DNDEBUG generated_main.cpp -o cpp-rinher-runner  -ljsoncpp -flto > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        ./cpp-rinher-runner
        exit $?
    fi
fi

./cpp-rinher-compiler $1 0

./julia-1.9.3/bin/julia generated_main.jl
#julia generated_main.jl
