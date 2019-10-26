#!/bin/bash
rm CMakeCache.txt
CXX=clang++ cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release .
make -j3
strip YappyCam.exe
strip finalize.exe
strip silent.exe
strip sound2wav.exe
