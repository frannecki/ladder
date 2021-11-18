#include <string>

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <utils.h>

#include <EventLoopThread.h>
#include <TcpClient.h>


using namespace ladder;
using namespace std::placeholders;

static int count = 0;

void OnConnection(const ConnectionPtr& conn) {
  LOGF_INFO("Current number of clients connected: %d", ++count);
  conn->Send("Hello -- This is a ladder ssl client.");
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
  Logger::create("./test_ssl_client.log");
  SocketAddr addr("127.0.0.1", 8080, false);
  EventLoopThread loop_thread;
  SslInit();
  TcpClient client(addr, loop_thread.loop(), 10, true);
  client.set_connection_callback(std::bind(OnConnection, _1));
  client.set_read_callback(std::bind(OnMessage, _1, _2));
  client.Connect();
  while (1)
    ;
  return 0;
}
