#include <Buffer.h>

namespace ladder {

Buffer::Buffer() : 
  read_index_(kPrependLength),
  write_index_(kPrependLength)
{
  ;
}

std::string Buffer::Read(size_t n) {
  std::string result;
  uint32_t readable  = std::min(static_cast<uint32_t>(n), ReadableBytes());
  result.assign(buffer_.data() + read_index_,
                buffer_.data() + read_index_ + readable);
  read_index_ += readable;
  if(write_index_ == read_index_) {
    write_index_ = read_index_ = kPrependLength;
  }
  return result;
}

std::string Buffer::ReadAll() {
  return Read(write_index_ - read_index_);
}

uint32_t Buffer::ReadableBytes() const {
  return write_index_ - read_index_;
}

uint32_t Buffer::ReadUInt32() {
  uint32_t result;
  std::string buf = Read(sizeof(result));
  memcpy(&result, buf.c_str(), sizeof(result));
  return result;
}

void Buffer::Write(const std::string& content) {
  uint32_t len = content.size();
  std::copy(content.begin(), content.end(), std::back_inserter(buffer_));
  write_index_ += len;
}

void Buffer::WriteUInt32(uint32_t number) {
  char buf_str[sizeof(number)];
  memcpy(buf_str, &number, sizeof(number));
  std::string buf(buf_str, sizeof(number));
  Write(buf);
}

void Buffer::Prepend(uint32_t number) {
  char buf_str[sizeof(number)];
  memcpy(buf_str, &number, sizeof(number));
  Prepend(buf_str, sizeof(number));
}

void Buffer::Prepend(const char* data, size_t len) {
  assert(len <= kPrependLength);
  memcpy(buffer_.data() + read_index_ - len, data, len);
}


} // namespace ladder
