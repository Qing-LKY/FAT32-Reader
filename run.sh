#!/bin/bash

cd build
cmake ..
make clean
make
./src/fat32-reader