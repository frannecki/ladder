# Ladder: A Light-weight Event-driven Server

Ladder is a light-weight web server framework developed for studying purposes. It conforms to the _Reactor_ design patterns and leverages Linux epoll / BSD kqueue / Windows IOCP for i/o multiplexing.

## Prerequisites
* protobuf 3.x
* zlib
* OpenSSL
* googletest 1.10.x (optional, for unit tests)

## Supported Platforms
* Linux
* FreeBSD
* Windows

## Usage
### Building

Buiding scripts with cmake and bazel are provided for Linux/FreeBSD. You can refer to [msvc](msvc) for its usage on Windows.

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
  bazel build --cxxopt="-std=c++11" //:ladder
  bazel build --cxxopt="-std=c++11" //:ladder_client
  bazel build --cxxopt="-std=c++11" //:ladder_unit_tests
  bazel build --cxxopt="-std=c++11" //:ladder_tests_logger
  bazel build --cxxopt="-std=c++11" //:ladder_tests_server
  bazel build --cxxopt="-std=c++11" //:ladder_tests_proto_server
  bazel build --cxxopt="-std=c++11" //:ladder_tests_event_poller
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_timer
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_client
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_mass_clients
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_tcp_client
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_event_loop_thread
  bazel build --cxxopt="-std=c++11" //:ladder_test_http_server
  ```

* cmake

  ```sh
  mkdir -p build && cd build
  cmake ..
  make
  ```

### Usage
To use the ladder library, please refer to the [test demo](tests/server) for more details.
