#include <TcpServer.h>

int main(int argc, char** argv) {
  ladder::SocketAddr addr("127.0.0.1", 8070);
  ladder::TcpServer server(addr, false);
  server.Start();
  return 0;
}
