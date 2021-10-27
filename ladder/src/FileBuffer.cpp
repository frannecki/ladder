#include <fcntl.h>

#include <Buffer.h>
#include <FileBuffer.h>
#include <Socket.h>

namespace ladder {

FileBuffer::FileBuffer()
    : buffer_(new Buffer), bytes_sent_(0), bytes_pending_(0), fd_(-1) {}

FileBuffer::~FileBuffer() {
  if (buffer_) delete buffer_;
  buffer_ = nullptr;
  if (fd_ != -1) {
    socket::close(fd_);
  }
  fd_ = -1;
}

void FileBuffer::AddFile(std::string&& header, const std::string& filename) {
  pending_files_.push(FileInfo(std::move(header), filename));
}

void FileBuffer::Write(const std::string& content) {
  AddFile(std::string(content), "");
}

int FileBuffer::WriteBufferToFd(int fd) {
  int ret = buffer_->WriteBufferToFd(fd);
  if (ret < 0) {
    return ret;
  }

  while (!pending_files_.empty()) {
    if (fd_ == -1) {
      buffer_->Write(pending_files_.front().header_);
      std::string& filename = pending_files_.front().filename_;
      if (!filename.empty()) {
        fd_ = open(pending_files_.front().filename_.c_str(), O_RDONLY);
      }
      if (fd_ != -1) {
        FILE* fp = fdopen(fd_, "r");
        fseek(fp, 0, SEEK_END);
        bytes_pending_ = ftell(fp);
        bytes_sent_ = 0;
        fseek(fp, 0, SEEK_SET);
      } else {
        pending_files_.pop();
      }
    }

    ret = buffer_->WriteBufferToFd(fd);
    if (ret < 0) {
      return ret;
    }

    if (fd_ != -1) {
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
          socket::close(fd_);
          fd_ = -1;
          pending_files_.pop();
        }
      }
    }
  }

  return 0;
}

bool FileBuffer::Empty() const { return pending_files_.empty(); }

}  // namespace ladder
