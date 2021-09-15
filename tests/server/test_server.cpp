#include <string>
#include <iostream>
#include <functional>

#include <TcpServer.h>
#include <Buffer.h>
#include <Socket.h>
#include <Connection.h>
#include <Logger.h>

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  LOG_INFO("Recv: " + message);
  conn->Send("Hello~ " + message);
}

void OnConnection(const ConnectionPtr& conn) {
  LOG_WARNING("Connection established");
  LOG_INFO("Current number of clients connected: " + std::to_string(++count));
  conn->Send("Hello -- This is a ladder server.");
}

int main(int argc, char** argv) {
  Logger::create();
  SocketAddr addr("127.0.0.1", 8070, false);
  // TcpServer server(addr, 100);
  TcpServer server(addr, 10);
  server.SetConnectionCallback(std::bind(OnConnection, _1));
  server.SetReadCallback(std::bind(OnMessage, _1, _2));
  server.Start();
  return 0;
}
