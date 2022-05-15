#include <functional>
#include <string>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <TcpServer.h>

using namespace ladder;
using namespace std::placeholders;

static int count = 0;
static TcpServer* server = nullptr;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  LOG_INFO("Recv: " + message);
  conn->Send("Hello~ " + message);
}

void OnConnection(const ConnectionPtr& conn) {
  LOGF_INFO("Current number of clients connected: %d", ++count);
  conn->Send("Hello -- This is a ladder server.");
  if (count >= 5) {
    server->Stop();
  }
}

int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    return -1;
  }
#endif
  Logger::create("./test_stop_server.log");
  SocketAddr addr("0.0.0.0", 8070, false);
  server = new TcpServer(addr, false);
#ifndef _MSC_VER
  server->SetConnectionCallback(std::bind(OnConnection, _1));
#endif
  server->SetReadCallback(std::bind(OnMessage, _1, _2));
  server->Start();
  LOG_WARNING("Server stopped");
  delete server;
  while (1);
  return 0;
}
