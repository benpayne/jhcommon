#!/bin/bash

mkdir -p Debug
cd Debug
cmake -D CMAKE_BUILD_TYPE=Debug ..

cd ..
mkdir -p Release
cd Release
cmake -D CMAKE_BUILD_TYPE=Release ..

