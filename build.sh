#!/bin/bash

mkdir -p Debug
cd Debug
cmake -D CMAKE_BUILD_TYPE=debug ..

cd ..
mkdir -p Release
cd Release
cmake -D CMAKE_BUILD_TYPE=debug ..
