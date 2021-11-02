#!/bin/bash
bazel build --cxxopt="-std=c++11" //:all
sudo cp bazel-bin/libladder.so /usr/local/lib
sudo cp bazel-bin/libladder_client.so /usr/local/lib
sudo cp -r ladder/include /usr/local/include/ladder
sudo cp -r ladder_client/include /usr/local/include/ladder_client
