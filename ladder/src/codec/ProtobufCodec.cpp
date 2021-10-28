#ifdef __unix__
#include <arpa/inet.h>
#elif defined(_MSC_VER)
#include <winsock.h>
#endif
#include <zlib.h>

#include <Buffer.h>
#include <Connection.h>
#include <codec/ProtobufCodec.h>

namespace ladder {

ProtobufCodec::~ProtobufCodec() {}

static google::protobuf::Message* CreateMessage(const std::string& type_name) {
  google::protobuf::Message* message = NULL;
  const google::protobuf::Descriptor* descriptor =
      google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(
          type_name);
  if (descriptor) {
    const google::protobuf::Message* prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(
            descriptor);
    if (prototype) {
      message = prototype->New();
    }
  }
  return message;
}

// message type name length
uint32_t ProtobufCodec::kMinMessageLength = sizeof(uint32_t);

void ProtobufCodec::ComposeMessage(const void* message,
                                   std::string& buf) const {
  const google::protobuf::Message* message_proto =
      reinterpret_cast<const google::protobuf::Message*>(message);
  Buffer buffer;
  std::string message_type_name = message_proto->GetTypeName();
  buffer.WriteUInt32(htonl(static_cast<uint32_t>(message_type_name.size())));
  buffer.Write(message_type_name);

  std::string message_buf = message_proto->SerializeAsString();
  buffer.Write(message_buf);

  buf = buffer.ReadAll();
}

bool ProtobufCodec::ParseMessage(const std::string& packet,
                                 void*& message) const {
  if (packet.size() < kMinMessageLength) {
    return false;
  }

  uint32_t message_type_name_length;
  memcpy(&message_type_name_length, packet.c_str(),
         sizeof(message_type_name_length));
  message_type_name_length = ntohl(message_type_name_length);
  if (packet.size() < message_type_name_length + kMinMessageLength) {
    return false;
  }

  std::string message_type_name =
      packet.substr(sizeof(message_type_name_length), message_type_name_length);
  google::protobuf::Message* message_proto = CreateMessage(message_type_name);
  if (message_proto == nullptr) {
    return false;
  }

  uint32_t message_buffer_len =
      packet.size() - kMinMessageLength - message_type_name_length;
  std::string message_buffer =
      packet.substr(sizeof(message_type_name_length) + message_type_name_length,
                    message_buffer_len);
  if (!message_proto->ParseFromString(message_buffer)) {
    return false;
  }

  message = reinterpret_cast<void*>(message_proto);

  return true;
}

bool ProtobufCodec::HandleMessage(const ConnectionPtr& conn,
                                  void* message) const {
  google::protobuf::Message* message_proto =
      reinterpret_cast<google::protobuf::Message*>(message);
  if (message) {
    auto iter = callbacks_.find(message_proto->GetDescriptor());
    if (iter != callbacks_.end()) {
      iter->second->OnMessage(conn, message_proto);
    } else {
      default_callback_(conn, message_proto);
    }
  }

  if (message_proto) {
    delete message_proto;
    message_proto = nullptr;
    message = nullptr;
  }

  return true;
}

void ProtobufCodec::RegisterDefaultMessageCallback(
    const std::function<void(const ConnectionPtr&, google::protobuf::Message*)>&
        callback) {
  default_callback_ = callback;
}

}  // namespace ladder
