# Ladder: A Light-weight Reactor Based Web Server

## Prerequisites
* protobuf 3.x
* zlib
* googletest 1.10.x (optional, for unit tests)

## Building
* GNU make

  ```sh
  # GNU make
  make
  ```

* bazel
  
  ```sh
  # build all
  bazel build --cxxopts="-std=c++11" //:all
  ```

  or alternatively

  ```sh
  bazel build --cxxopts="-std=c++11" //:ladder
  bazel build --cxxopts="-std=c++11" //:ladder_client
  bazel build --cxxopts="-std=c++11" //:ladder_unit_tests
  bazel build --cxxopts="-std=c++11" //:ladder_tests_logger
  bazel build --cxxopts="-std=c++11" //:ladder_tests_server
  bazel build --cxxopts="-std=c++11" //:ladder_tests_proto_server
  bazel build --cxxopts="-std=c++11" //:ladder_tests_event_poller
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_event_client
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_mass_clients
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_tcp_client
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_event_loop_thread
  ```

## Usage
