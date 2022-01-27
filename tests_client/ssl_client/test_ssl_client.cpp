#include <string>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <utils.h>

#include <EventLoopThread.h>
#include <TcpClient.h>

using namespace ladder;
using namespace std::placeholders;


void OnConnection(const ConnectionPtr& conn) {
  conn->Send("Hello -- This is a ladder ssl client.");
}

void OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string buf = buffer->ReadAll();
  LOG_INFO("Recv: " + buf);
  std::string message_back = "Echoback: " + buf;
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
  Logger::create("./test_ssl_client.log");
  SocketAddr addr("127.0.0.1", 8070, false);
  EventLoopThread loop_thread;
  SslInit();
  TcpClient client(addr, loop_thread.loop(), 10, true);
  client.SetConnectionCallback(std::bind(OnConnection, _1));
  client.SetReadCallback(std::bind(OnMessage, _1, _2));
#ifdef _MSC_VER
  SocketAddr local_addr("127.0.0.1", 31312, false);
  client.Connect(local_addr);
#else
  client.Connect();
#endif
  while (1)
    ;
  return 0;
}
