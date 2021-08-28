#include <unistd.h>

#include <Buffer.h>
#include <utils.h>

namespace ladder {

const uint32_t kPrependLength = 8;
const uint32_t kReadBufferSize = 1024;
const uint32_t kWriteBufferSize = 1024;

Buffer::Buffer() : 
  read_index_(kPrependLength),
  write_index_(kPrependLength)
{
  ;
}

std::string Buffer::Read(size_t n) {
  std::string result;
  uint32_t readable = Peek(n, result);
  HaveRead(readable);
  return result;
}

uint32_t Buffer::Peek(size_t n, std::string& result) {
  uint32_t readable  = std::min(static_cast<uint32_t>(n), ReadableBytes());
  result.assign(buffer_.data() + read_index_,
                buffer_.data() + read_index_ + readable);
  return readable;
}

void Buffer::HaveRead(size_t n) {
  std::lock_guard<std::mutex> lock(mutex_);
  read_index_ += std::min(static_cast<uint32_t>(n), ReadableBytes());
  if(write_index_ == read_index_) {
    write_index_ = read_index_ = kPrependLength;
  }
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
  {
    std::lock_guard<std::mutex> lock(mutex_);
    write_index_ += len;
  }
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

int Buffer::ReadBufferFromFd(int fd) {
  int ret = 0;
  char buf[kReadBufferSize];
  while(1) {
    ret = ::read(fd, buf, sizeof(buf));
    if(ret == -1) {
      switch(errno) {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
          break;
        default:
          EXIT("[Buffer] read");
      }
    }
    else {
      Write(std::string(buf, buf+ret));
    }
  }
  return ret;
}

void Buffer::WriteBufferToFd(int fd) {
  std::string buf;
  while(ReadableBytes() > 0) {
    uint32_t read_len = Peek(kWriteBufferSize, buf);
    int ret = ::write(fd, buf.c_str(), buf.size());
    if(ret == -1) {
      switch(errno) {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
          break;
        default:
          EXIT("[Buffer] write");
      }
    }
    else {
      HaveRead(static_cast<size_t>(read_len));
    }
  }
}


} // namespace ladder
