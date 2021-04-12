#!/bin/bash
rm CMakeCache.txt
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release .
make -j3
