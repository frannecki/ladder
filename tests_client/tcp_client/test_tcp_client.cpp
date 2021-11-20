#include <string>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#include <Buffer.h>
#include <Connection.h>

#include <EventLoopThread.h>
#include <Logging.h>
#include <TcpClient.h>

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnConnection(const ConnectionPtr& conn) {
  conn->Send("Hello~ Greetings from ladder tcp client.");
}

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string buf = buffer->ReadAll();
  LOG_INFO("Recv: " + buf);
  LOGF_INFO("Current count value: %d %u", count, buf.size());
  std::string message_back = "Echoback" + std::to_string(++count) + ": " + buf;
  LOGF_INFO("Length of message back: %u", message_back.size());
  conn->Send(message_back);
}

int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    return -1;
  }
#endif
  Logger::create("./test_tcp_client.log");
  SocketAddr addr("127.0.0.1", 8070, false);
  EventLoopThread loop_thread;
  TcpClient client(addr, loop_thread.loop(), 10);
  client.set_read_callback(std::bind(OnMessage, _1, _2));
#ifdef _MSC_VER
  client.set_connection_callback(std::bind(OnConnection, _1));
  SocketAddr local_addr("127.0.0.1", 31311, false);
  client.Connect(local_addr);
#else
  client.Connect();
#endif
  while (1)
    ;
  return 0;
}
