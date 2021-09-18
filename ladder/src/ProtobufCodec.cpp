#include <endian.h>
#include <zlib.h>

#include <ProtobufCodec.h>
#include <Connection.h>
#include <Buffer.h>

namespace ladder {

static google::protobuf::Message* CreateMessage(const std::string& type_name)
{
  google::protobuf::Message* message = NULL;
  const google::protobuf::Descriptor* descriptor =
    google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
  if (descriptor)
  {
    const google::protobuf::Message* prototype =
      google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
    if (prototype)
    {
      message = prototype->New();
    }
  }
  return message;
}

ProtobufCodec::ProtobufCodec() {

}

uint32_t ProtobufCodec::kMinMessageLength = \
  sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

void ProtobufCodec::Send(const ConnectionPtr& conn,
                         google::protobuf::Message* message)
{
  std::string buf;
  ComposeMessage(message, buf);
  conn->Send(buf);
}

void ProtobufCodec::OnMessage(const ConnectionPtr& conn,
                              Buffer* buffer)
{
  google::protobuf::Message* message = nullptr;
  
  int length = ParseMessage(message, buffer);
  
  if(length >= static_cast<int>(kMinMessageLength)) {
    auto iter = callbacks_.find(
      const_cast<google::protobuf::Descriptor*>(message->GetDescriptor()));
    if(iter != callbacks_.end()) {
      iter->second->OnMessage(conn, message);
    }

    buffer->HaveRead(length);
  }

  if(message) {
    delete message;
    message = nullptr;
  }
}

void ProtobufCodec::ComposeMessage(google::protobuf::Message* message, std::string& buf) {
  Buffer buffer;
  std::string message_type_name = message->GetTypeName();
  buffer.WriteUInt32(htobe32(static_cast<uint32_t>(message_type_name.size())));
  buffer.Write(message_type_name);

  std::string message_buf = message->SerializeAsString();
  buffer.Write(message_buf);

  uint32_t length = kMinMessageLength + message_type_name.size() + message_buf.size();
  buffer.Prepend(htobe32(length));

  buf = buffer.RetrieveAll();
  uint32_t checksum = static_cast<uint32_t>(crc32(0L, Z_NULL, 0));
  checksum = static_cast<uint32_t>(crc32(
    checksum,
    reinterpret_cast<const unsigned char*>(buf.c_str()),
    buf.size()));
  checksum = htobe32(checksum);
  char checksum_buf[sizeof(checksum)];
  memcpy(checksum_buf, &checksum, sizeof(checksum));

  std::string checksum_str(checksum_buf, checksum_buf + sizeof(checksum));
  std::copy(checksum_str.begin(), checksum_str.end(), std::back_inserter(buf));
}

int ProtobufCodec::ParseMessage(google::protobuf::Message*& message, Buffer* buffer) {
  if(buffer->ReadableBytes() < kMinMessageLength) {
    return -1;
  }
  
  uint32_t length = be32toh(buffer->PeekUInt32());
  if(buffer->ReadableBytes() < length) {
    return -1;
  }

  std::string buf;
  buffer->Peek(length, buf);
  
  uint32_t message_type_name_length;
  memcpy(&message_type_name_length,
         buf.c_str() + sizeof(length),
         sizeof(message_type_name_length));
  message_type_name_length = be32toh(message_type_name_length);
  if(length < message_type_name_length + kMinMessageLength) {
    return -1;
  }

  // checksum
  uint32_t checksum_expected = static_cast<uint32_t>(crc32(0L, Z_NULL, 0));
  checksum_expected = static_cast<uint32_t>(crc32(
    checksum_expected,
    reinterpret_cast<const unsigned char*>(buf.c_str()),
    buf.size()-sizeof(checksum_expected)));
  uint32_t checksum;
  memcpy(&checksum, buf.c_str() + length - sizeof(checksum), sizeof(checksum));
  checksum = be32toh(checksum);
  if(checksum != checksum_expected) {
    return -1;
  }

  std::string message_type_name = buf.substr(
    sizeof(length) + sizeof(message_type_name_length),
    message_type_name_length);
  message = CreateMessage(message_type_name);
  if(message == nullptr) {
    return -1;
  }

  uint32_t message_buffer_len = length - \
    kMinMessageLength - message_type_name_length;
  std::string message_buffer = buf.substr(
    sizeof(length) + sizeof(message_type_name_length) \
      + message_type_name_length,
    message_buffer_len);
  if(!message->ParseFromString(message_buffer)) {
    return -1;
  }

  return length;
}

void ProtobufCodec::RegisterDefaultMessageCallback(
  const std::function<void(const ConnectionPtr&,
  google::protobuf::Message*)>& callback)
{
  default_callback_ = callback;
}

} // namespace ladder
