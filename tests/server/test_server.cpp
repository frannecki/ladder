#include <functional>
#include <string>

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <Socket.h>
#include <TcpServer.h>

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  LOG_INFO("Recv: " + message);
  conn->Send("Hello~ " + message);
}

void OnConnection(const ConnectionPtr& conn) {
  LOGF_INFO("Current number of clients connected: %d", ++count);
  conn->Send("Hello -- This is a ladder server.");
}

int main(int argc, char** argv) {
  Logger::create("./test_server.log");
  SocketAddr addr("0.0.0.0", 8070, false);
  TcpServer server(addr, false);
  server.SetConnectionCallback(std::bind(OnConnection, _1));
  server.SetReadCallback(std::bind(OnMessage, _1, _2));
  server.Start();
  return 0;
}
