#ifdef __unix__
#include <unistd.h>
#endif
#ifdef _MSC_VER
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
#ifdef _MSC_VER
  uint32_t readable = min(static_cast<uint32_t>(n), ReadableBytes());
#else
  uint32_t readable = std::min(static_cast<uint32_t>(n), ReadableBytes());
#endif
  result.assign(buffer_.begin() + read_index_,
                buffer_.begin() + read_index_ + readable);
  return readable;
}

void Buffer::HaveRead(size_t n) {
  std::lock_guard<std::mutex> lock(mutex_);
#ifdef _MSC_VER
  read_index_ += min(static_cast<uint32_t>(n), ReadableBytes());
#else
  read_index_ += std::min(static_cast<uint32_t>(n), ReadableBytes());
#endif
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
  std::copy(content.begin(), content.end(),
            // std::back_inserter(buffer_));
            std::inserter(buffer_, buffer_.begin() + write_index_));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    write_index_ += content.size();
  }
}

void Buffer::Write(const char* src, size_t len) {
  std::copy(src, src + len,
            std::inserter(buffer_, buffer_.begin() + write_index_));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    write_index_ += len;
  }
}

void Buffer::WriteUInt32(uint32_t number) {
  char buf_str[sizeof(number)];
  memcpy(buf_str, &number, sizeof(number));
  Write(buf_str, sizeof(number));
}

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

}  // namespace ladder
