#include <functional>
#include <string>

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <TcpServer.h>
#include <utils.h>

#ifdef LADDER_OS_WINDOWS
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  LOG_INFO("Recv: " + message);
  conn->Send(
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
      "26\r\n\r\nHello this is SslTcpServer");
}

int main(int argc, char** argv) {
#ifdef LADDER_OS_WINDOWS
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    return -1;
  }
#endif
  if (argc < 3) {
    fprintf(stderr, "Usage: %s $CertPath $KeyPath\n", argv[0]);
    return EXIT_FAILURE;
  }
  Logger::create("./test_ssl_server.log");
  SocketAddr addr("0.0.0.0", 8070, false);
  SslInit();
  TcpServer server(addr, false, argv[1], argv[2]);
  server.set_read_callback(std::bind(OnMessage, _1, _2));
  server.Start();
  return EXIT_SUCCESS;
}
