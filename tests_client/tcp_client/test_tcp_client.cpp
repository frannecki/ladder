#include <string>

#include <TcpClient.h>
#include <EventLoopThread.h>
#include <Buffer.h>
#include <Connection.h>

#include <Logger.h>

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string buf = buffer->ReadAll();
  LOG_INFO("Recv: " + buf);
  LOG_INFO("Current count value: " + std::to_string(count) + " " + std::to_string(buf.size()));
  std::string message_back = "Echoback" + std::to_string(++count) + ": " + buf;
  LOG_INFO("Length of message back: " + std::to_string(message_back.size()));
  conn->Send(message_back);
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
