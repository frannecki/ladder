# Ladder: A Light-weight Event-driven Server

Ladder is a light-weight web server framework developed for studying purposes. It conforms to the _Reactor_ design patterns and leverages Linux epoll / BSD kqueue / Windows IOCP for I/O multiplexing or asynchronous IO.

## Required Dependencies
* protobuf 3.x
* zlib
* OpenSSL

## Supported Platforms
* Linux
* FreeBSD
* Windows
* macOS

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
  bazel build --cxxopt="-std=c++11" //:ladder_tests_file_server
  bazel build --cxxopt="-std=c++11" //:ladder_tests_ssl_server

  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_timer
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_client
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_mass_clients
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_tcp_client
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_event_loop_thread
  bazel build --cxxopt="-std=c++11" //:ladder_tests_client_ssl_client

  bazel build --cxxopt="-std=c++11" //:ladder_test_http_server
  ```

* cmake

  ```sh
  ./build_cmake.sh
  ```

  or alternatively

  ```sh
  mkdir build-cmake
  protoc -I=tests/proto --cpp_out=tests/proto tests/proto/*.proto || exit 1
  mkdir -p build-cmake && cd build-cmake
  cmake .. && make
  make install # Optional. Installation path defaults to `/usr/local/include` and `/usr/local/lib`
  ```

### Usage
#### Start a ladder server
```cpp
#include <functional>
#include <string>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#include <ladder/Buffer.h>
#include <ladder/Connection.h>
#include <ladder/TcpServer.h>
#include <ladder/Logging.h>

using namespace ladder;
using namespace std::placeholders;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  LOG_INFO("Received: " + message);
  conn->Send("Echoback~ " + message);
}

void OnConnection(const ConnectionPtr& conn) {
  conn->Send("Hello -- This is a ladder server.");
}

int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    return -1;
  }
#endif
  SocketAddr addr("0.0.0.0", 8070, false);
  TcpServer server(addr, false);
  server.SetConnectionCallback(std::bind(OnConnection, _1));
  server.SetReadCallback(std::bind(OnMessage, _1, _2));
  server.Start();
#ifdef _MSC_VER
  WSACleanup();
#endif
  return 0;
}
```

You can refer to the [test demo](tests/server) for more details.
