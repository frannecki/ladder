#include <string>
#include <functional>

#include <TcpServer.h>
#include <Buffer.h>
#include <Socket.h>
#include <Connection.h>
#include <ProtobufCodec.h>
#include <Logger.h>

#include "proto/tests.pb.h"

using namespace ladder;
using namespace std::placeholders;

static int count = 0;

using TestMessage1Ptr = std::shared_ptr<TestMessage1>;

void OnMessage(const ConnectionPtr& conn, TestMessage1* message, const ProtobufCodec& codec) {
  
  LOG_INFO("Received message: " + message->GetTypeName());
  LOG_INFO("message key: " + message->key());
  LOG_INFO("message value: " + message->value());
  LOGF_INFO("success: %d", message->success());
  LOGF_INFO("message validate: %d", message->validate());
  LOGF_INFO("message id: %u", message->id());

  message->set_id(message->id() + 1);
  codec.Send(conn, message);
}

void OnDefaultMessage(const ConnectionPtr& conn, google::protobuf::Message* message) {
  LOG_WARNING("unrecognized message type: " + message->GetTypeName());
}

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

int main(int argc, char** argv) {
  Logger::create();
  SocketAddr addr("0.0.0.0", 8070, false);
  TcpServer server(addr, 1);
  ProtobufCodec codec;
  codec.RegisterMessageCallback<ladder::TestMessage1>(std::bind(OnMessage, _1, _2, codec));
  codec.RegisterDefaultMessageCallback(std::bind(OnDefaultMessage, _1, _2));
  server.SetConnectionCallback(std::bind(OnConnection, _1, codec));
  server.SetReadCallback(std::bind(&ProtobufCodec::OnMessage, &codec, _1, _2));
  server.Start();
  return 0;
}
