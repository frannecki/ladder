#ifdef __unix__
#include <arpa/inet.h>
#elif defined(_MSC_VER)
#include <winsock2.h>
#endif
#include <zlib.h>

#include <Buffer.h>
#include <Connection.h>
#include <codec/Codec.h>

namespace ladder {

Codec::~Codec() {}

const uint32_t Codec::kHeaderLength = sizeof(uint32_t) + sizeof(uint32_t);

void Codec::Send(const ConnectionPtr& conn, const void* message) const {
  std::string packet;
  Encapsulate(message, packet);
  conn->Send(packet);
}

void Codec::OnMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string packet;
  if (Decapsulate(packet, buffer)) {
    ParseAndHandleMessage(conn, packet);
  }
}

void Codec::Encapsulate(const void* message, std::string& packet) const {
  std::string raw_data;
  ComposeMessage(message, raw_data);

  uint32_t length = kHeaderLength + raw_data.size();
  length = htonl(length);

  uint32_t checksum = static_cast<uint32_t>(crc32(0L, Z_NULL, 0));
  checksum = static_cast<uint32_t>(
      crc32(checksum, reinterpret_cast<const unsigned char*>(raw_data.c_str()),
            raw_data.size()));
  checksum = htonl(checksum);

  // compose header
  char header_str[kHeaderLength];
  memcpy(header_str, &length, sizeof(uint32_t));
  memcpy(header_str + sizeof(uint32_t), &checksum, sizeof(uint32_t));

  packet = std::string(header_str, header_str + kHeaderLength) + raw_data;
}

bool Codec::Decapsulate(std::string& raw_data, Buffer* buffer) const {
  if (buffer->ReadableBytes() < kHeaderLength) {
    return false;
  }

  uint32_t length = ntohl(buffer->PeekUInt32());
  if (buffer->ReadableBytes() < length) {
    return false;
  }

  std::string buf;
  buffer->Peek(length, buf);
  buffer->HaveRead(length);

  // checksum
  uint32_t checksum_expected = static_cast<uint32_t>(crc32(0L, Z_NULL, 0));
  checksum_expected = static_cast<uint32_t>(
      crc32(checksum_expected,
            reinterpret_cast<const unsigned char*>(buf.c_str() + kHeaderLength),
            buf.size() - kHeaderLength));
  uint32_t checksum;
  memcpy(&checksum, buf.c_str() + sizeof(length), sizeof(checksum));
  checksum = ntohl(checksum);
  if (checksum != checksum_expected) {
    return false;
  }

  raw_data = buf.substr(kHeaderLength, buf.size() - kHeaderLength);

  return true;
}

void Codec::ParseAndHandleMessage(const ConnectionPtr& conn,
                                  const std::string& packet) const {
  void* message = nullptr;
  if (ParseMessage(packet, message) && message != nullptr) {
    // `message` freed in HandleMessage
    HandleMessage(conn, message);
    message = nullptr;
  }
}

}  // namespace ladder
