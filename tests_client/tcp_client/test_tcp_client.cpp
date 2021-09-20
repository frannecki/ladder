#include <string>

#include <TcpClient.h>
#include <EventLoopThread.h>
#include <Buffer.h>
#include <Connection.h>

#include <Logger.h>
// #include <iostream>
// #define LOG_INFO(msg) std::cout << msg << std::endl;


using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string buf = buffer->ReadAll();
  // LOG_INFO("Recv: " + buf);
  LOG_INFO("Current count value: " + std::to_string(count));
  // conn->Send("Echoback" + std::to_string(++count) + ": " + buf);
  conn->Send(buf);
}

int main(int argc, char** argv) {
  Logger::create("./test_tcp_client.log");
  SocketAddr addr("127.0.0.1", 8070, false);
  EventLoopThread loop_thread;
  TcpClient client(addr, loop_thread.loop(), 10);
  client.SetReadCallback(std::bind(OnMessage, _1, _2));
  client.Connect();
  while(1);
  return 0;
}
