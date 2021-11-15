#include <functional>
#include <string>

#include <Buffer.h>
#include <Connection.h>
#include <Logging.h>
#include <Socket.h>
#include <TcpServer.h>
#include <codec/ProtobufCodec.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#include "proto/tests.pb.h"

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

using TestMessage1Ptr = std::shared_ptr<TestMessage1>;

#ifdef _MSC_VER
static ProtobufCodec* p_codec = nullptr;

void OnRawMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string msg = buffer->ReadAll();
  LOG_INFO("Recv: " + msg);
  // conn->Send("Recv~ " + msg);
  conn->set_read_callback(
      std::bind(&ProtobufCodec::OnMessage, p_codec, _1, _2));

  ladder::TestMessage1 message;
  message.set_key("arbitrary_key");
  message.set_value("arbitrary_value");
  message.set_success(false);
  message.set_validate(true);
  message.set_id(9);
  p_codec->Send(conn, &message);
}
#else
void OnConnection(const ConnectionPtr& conn, const ProtobufCodec& codec) {
  LOGF_INFO("Current number of clients connected: %d", ++count);
  ladder::TestMessage1 message;
  message.set_key("arbitrary_key");
  message.set_value("arbitrary_value");
  message.set_success(false);
  message.set_validate(true);
  message.set_id(9);
  codec.Send(conn, &message);
}
#endif

void OnMessage(const ConnectionPtr& conn, TestMessage1* message,
               const ProtobufCodec& codec) {
  LOG_INFO("Received message: " + message->GetTypeName());
  LOG_INFO("message key: " + message->key());
  LOG_INFO("message value: " + message->value());
  LOGF_INFO("success: %d", message->success());
  LOGF_INFO("message validate: %d", message->validate());
  LOGF_INFO("message id: %u", message->id());

  message->set_id(message->id() + 1);
  codec.Send(conn, message);
}

void OnDefaultMessage(const ConnectionPtr& conn,
                      google::protobuf::Message* message) {
  LOG_WARNING("unrecognized message type: " + message->GetTypeName());
}

int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    return -1;
  }
#endif
  Logger::create();
  SocketAddr addr("0.0.0.0", 8070, false);
  TcpServer server(addr, false);
  ProtobufCodec codec;
  codec.RegisterMessageCallback<ladder::TestMessage1>(
      std::bind(OnMessage, _1, _2, codec));
  codec.RegisterDefaultMessageCallback(std::bind(OnDefaultMessage, _1, _2));
#ifdef _MSC_VER
  // iocp server cannot send data on connection
  p_codec = &codec;
  server.set_read_callback(std::bind(OnRawMessage, _1, _2));
#else
  server.set_connection_callback(std::bind(OnConnection, _1, codec));
  server.set_read_callback(
      std::bind(&ProtobufCodec::OnMessage, &codec, _1, _2));
#endif
  server.Start();
  return 0;
}
