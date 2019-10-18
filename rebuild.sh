#!/bin/bash
rm CMakeCache.txt
CXX=clang++ cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release .
make -j3
strip finalize.exe
strip sound2wav.exe
strip YappyCam.exe
