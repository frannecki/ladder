# Ladder: A Light-weight Event-driven Server

## Prerequisites
* protobuf 3.x
* zlib
* OpenSSL
* googletest 1.10.x (optional, for unit tests)

## Supported Platforms
* Linux
* FreeBSD

## Usage
### Building
* GNU make

  ```sh
  ./build.sh
  ```

* bazel
  
  ```sh
  # build all
  sudo ./build_bazel.sh
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
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_timer
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_client
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_mass_clients
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_tcp_client
  bazel build --cxxopts="-std=c++11" //:ladder_tests_client_event_loop_thread
  bazel build --cxxopts="-std=c++11" //:ladder_test_http_server
  ```

* cmake

  ```sh
  mkdir -p build && cd build
  cmake ..
  make
  ```

### Usage
To use the ladder library, please refer to the [test demo](tests/server) for more details.

> Note: To run the http/https server demo `test_http_server` in https mode, directories to ssl certificate and key should be specified. i.e. `./test_http_server $IndexPath $CertPath $KeyPath`
