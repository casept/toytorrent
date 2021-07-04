#!/usr/bin/env sh
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DENABLE_SANITIZERS=OFF -DENABLE_WARNINGS=ON -DFORCE_COLORED_OUTPUT=ON .. && ninja -j4 && ./toytorrent_test
