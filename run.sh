#!/bin/bash

./cpp-rinher-compiler $1

/usr/bin/c++ -O3 -DNDEBUG out.cpp -o cpp-rinher-runner  -ljsoncpp

./cpp-rinher-runner