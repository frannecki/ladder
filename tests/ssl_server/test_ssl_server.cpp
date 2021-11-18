#include <functional>
#include <string>

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <Socket.h>
#include <TcpServer.h>
#include <utils.h>

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  LOG_INFO("Recv: " + message);
  conn->Send("Hello~ " + message);
}

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s $CertPath $KeyPath\n", argv[0]);
    return EXIT_FAILURE;
  }
  Logger::create("./test_ssl_server.log");
  SocketAddr addr("0.0.0.0", 8080, false);
  SslInit();
  TcpServer server(addr, false, argv[1], argv[2]);
  server.set_read_callback(std::bind(OnMessage, _1, _2));
  server.Start();
  return EXIT_SUCCESS;
}
