#!/bin/bash
protoc -I=tests/proto --cpp_out=tests/proto tests/proto/*.proto || exit 1
mkdir -p build-cmake && cd build-cmake
cmake .. && make
