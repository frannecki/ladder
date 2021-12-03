#!/bin/bash

## build libladder.so
cd ladder && make && cd .. || exit 1

## build libladder_client.so
cd ladder_client && make && cd .. || exit 1

## build ladder tests
cd tests
cd proto && protoc -I=. --cpp_out=. *.proto && cd .. || exit 1
cd unittests && make && cd .. || exit 1
./build.sh && cd .. || exit 1

cd tests_client && ./build.sh && cd .. || exit 1

## build http server
cd examples/http && make && cd .. || exit 1
