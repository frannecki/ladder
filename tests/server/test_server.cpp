#include <string>
#include <iostream>
#include <functional>

#include <TcpServer.h>
#include <Buffer.h>
#include <Socket.h>
#include <Logger.h>

using namespace ladder;
using namespace std::placeholders;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  std::cout << "Recv: " << message << std::cout;
  buffer->Write("Hello~ " + message);
}

void OnConnection(const ConnectionPtr& conn) {
  ;
}

int main(int argc, char** argv) {
  Logger::create();
  SocketAddr addr("127.0.0.1", 8070, false);
  TcpServer server(addr);
  server.SetConnectionCallback(std::bind(OnConnection, _1));
  server.SetReadCallback(std::bind(OnMessage, _1, _2));
  server.Start();
  return 0;
}
