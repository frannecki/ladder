#include <compat.h>
#ifdef LADDER_OS_UNIX
#include <unistd.h>
#endif
#ifdef LADDER_OS_WINDOWS
#include <winsock2.h>
#endif

#include <iterator>

#include <Buffer.h>
#include <Socket.h>
#include <utils.h>

namespace ladder {

const uint32_t kReadBufferSize = 1024;
const uint32_t kWriteBufferSize = 1024;

Buffer::Buffer() : read_index_(0), write_index_(0) {}

std::string Buffer::Read(size_t n) {
  std::string result;
  uint32_t readable = Peek(n, result);
  HaveRead(readable);
  return result;
}

uint32_t Buffer::Peek(size_t n, std::string& result) {
  uint32_t readable = (std::min)(static_cast<uint32_t>(n), ReadableBytes());
  result.assign(buffer_.begin() + read_index_,
                buffer_.begin() + read_index_ + readable);
  return readable;
}

uint32_t Buffer::Peek(char* dst, size_t n) {
  uint32_t readable = (std::min)(static_cast<uint32_t>(n), ReadableBytes());
  memcpy(dst, buffer_.data() + read_index_, readable);
  return readable;
}

void Buffer::HaveRead(size_t n) {
  std::lock_guard<std::mutex> lock(mutex_);
  read_index_ += (std::min)(static_cast<uint32_t>(n), ReadableBytes());
  if (write_index_ == read_index_) {
    write_index_ = read_index_ = 0;
  }
}

std::string Buffer::ReadAll() { return Read(write_index_ - read_index_); }

uint32_t Buffer::ReadableBytes() const { return write_index_ - read_index_; }

uint32_t Buffer::ReadUInt32() {
  uint32_t result;
  std::string buf = Read(sizeof(result));
  memcpy(&result, buf.c_str(), sizeof(result));
  return result;
}

uint32_t Buffer::PeekUInt32() {
  uint32_t result;
  std::string buf;
  Peek(sizeof(result), buf);
  memcpy(&result, buf.c_str(), sizeof(result));
  return result;
}

bool Buffer::Empty() const { return read_index_ == write_index_; }

void Buffer::Write(const std::string& content) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::copy(content.begin(), content.end(),
            std::inserter(buffer_, buffer_.begin() + write_index_));
  write_index_ += content.size();
}

void Buffer::Write(const char* src, size_t len) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::copy(src, src + len,
            std::inserter(buffer_, buffer_.begin() + write_index_));
  write_index_ += len;
}

void Buffer::WriteUInt32(uint32_t number) {
  char buf_str[sizeof(number)];
  memcpy(buf_str, &number, sizeof(number));
  Write(buf_str, sizeof(number));
}

#ifndef LADDER_OS_WINDOWS
int Buffer::ReadBufferFromFd(int fd) {
  int ret = 0;
  char buf[kReadBufferSize];
  while (1) {
    ret = socket::read(fd, buf, kReadBufferSize);
    if (ret < 0) {
      switch (errno) {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        case ECONNRESET:
          break;
        default:
          EXIT("[Buffer] read");
      }
    } else {
      Write(buf, ret);
    }
    if (ret <= 0) {
      break;
    }
  }
  return ret;
}

int Buffer::WriteBufferToFd(int fd) {
  std::string buf;
  while (ReadableBytes() > 0) {
    Peek(kWriteBufferSize, buf);
    int ret = socket::write(fd, buf.c_str(), buf.size());
    if (ret < 0) {
      switch (errno) {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        case ECONNRESET:
        case EPIPE:
          break;
        default:
          EXIT("[Buffer] write");
      }
    } else {
      HaveRead(static_cast<size_t>(ret));
    }
    if (ret < 0) {
      return ret;
    }
  }
  return 0;
}
#endif

}  // namespace ladder
