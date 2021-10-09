#!/bin/bash

## build libladder.so
cd ladder && make && cd ..

## build libladder_client.so
cd ladder_client && make && cd ..

## build ladder tests
cd tests
cd proto && protoc -I=. --cpp_out=. *.proto && cd ..
cd unittests && make && cd ..
./build.sh && cd ..

cd tests_client && ./build.sh && cd ..

## build http server
cd examples/http && make && cd ..
