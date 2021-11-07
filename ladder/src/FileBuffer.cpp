#include <fcntl.h>

#include <Buffer.h>
#include <FileBuffer.h>
#include <Socket.h>

namespace ladder {

FileBuffer::FileBuffer()
    : buffer_(new Buffer), bytes_sent_(0), bytes_pending_(0), fd_(0) {}

FileBuffer::~FileBuffer() {
  if (buffer_) delete buffer_;
  buffer_ = nullptr;
  if (fd_ != 0) {
#ifdef __unix__
    ::close(fd_);
#elif defined(_MSC_VER)
    CloseHandle(fd_);
#endif
  }
  fd_ = 0;
}

void FileBuffer::AddFile(std::string&& header, const std::string& filename) {
  pending_files_.push(FileInfo(std::move(header), filename));
}

void FileBuffer::Write(const std::string& content) {
  AddFile(std::string(content), "");
}

void FileBuffer::Write(const char* src, size_t len) {
  AddFile(std::string(src, len), "");
}

#ifndef _MSC_VER
int FileBuffer::WriteBufferToFd(int fd) {
  int ret = buffer_->WriteBufferToFd(fd);
  if (ret < 0) {
    return ret;
  }

  while (!pending_files_.empty()) {
    if (fd_ == 0) {
      buffer_->Write(pending_files_.front().header_);
      std::string& filename = pending_files_.front().filename_;
#ifdef __unix__
      if (!filename.empty()) {
        fd_ = open(pending_files_.front().filename_.c_str(), O_RDONLY);
        if (fd_ == -1) fd_ = 0;
      }
      if (fd_ != 0) {
        FILE* fp = fdopen(fd_, "r");
        fseek(fp, 0, SEEK_END);
        bytes_pending_ = ftell(fp);
        bytes_sent_ = 0;
        fseek(fp, 0, SEEK_SET);
      } else {
        pending_files_.pop();
      }
#elif defined(_MSC_VER)
      if (!filename.empty()) {
        fd_ = CreateFile((LPCWSTR)pending_files_.front().filename_.c_str(),
                         GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fd_ == INVALID_HANDLE_VALUE) fd_ = 0;
      }
      if (fd_ != 0) {
        DWORD sz = ::GetFileSize(fd_, nullptr);
        bytes_pending_ = static_cast<int>(sz);
        bytes_sent_ = 0;
      } else {
        pending_files_.pop();
      }
#endif
    }

    ret = buffer_->WriteBufferToFd(fd);
    if (ret < 0) {
      return ret;
    }

    if (fd_ != 0) {
      off_t offset = bytes_sent_;
      ret = socket::sendfile(fd, fd_, &offset, bytes_pending_);
      if (ret < 0) {
        switch (errno) {
          case EAGAIN:
#if EWOULDBLOCK != EAGAIN
          case EWOULDBLOCK:
#endif
          case EPIPE:
            break;
          default:
            EXIT("[FileBuffer] write");
        }
        return ret;
      } else {
        bytes_sent_ += ret;
        bytes_pending_ -= ret;
        if (bytes_pending_ == 0) {
#ifdef __unix__
          ::close(fd_);
#elif defined(_MSC_VER)
          CloseHandle(fd_);
#endif
          fd_ = 0;
          pending_files_.pop();
        }
      }
    }
  }

  return 0;
}
#endif

uint32_t FileBuffer::Peek(char* dst, size_t len) { return buffer_->Peek(dst, len); }

bool FileBuffer::Empty() const { return pending_files_.empty(); }

void FileBuffer::HaveRead(size_t n) {}

}  // namespace ladder
