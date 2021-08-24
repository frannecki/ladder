#include <string>
#include <iostream>
#include <functional>

#include <TcpServer.h>
#include <Buffer.h>

using namespace ladder;
using namespace std::placeholders;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message = buffer->ReadAll();
  std::cout << "Recv: " << message << std::cout;
  buffer->Write("Hello~ " + message);
}

int main(int argc, char** argv) {
  SocketAddr addr("127.0.0.1", 8070);
  TcpServer server(addr, false);
  server.SetReadCallback(std::bind(OnMessage, _1, _2));
  server.Start();
  return 0;
}
