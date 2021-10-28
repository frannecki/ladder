#ifdef __unix__
#include <unistd.h>
#endif

#include <functional>

#include <Buffer.h>
#include <Channel.h>
#include <Connection.h>
#include <EventLoop.h>
#include <FileBuffer.h>
#include <Socket.h>

namespace ladder {

Connection::Connection(const EventLoopPtr& loop, int fd, bool send_file)
    : channel_(new Channel(loop, fd)),
      read_buffer_(new Buffer),
      write_plain_buffer_(nullptr),
      write_file_buffer_(nullptr),
      shut_down_(false),
      send_file_(send_file) {
  if (send_file) {
    write_buffer_ = write_file_buffer_ = new FileBuffer;
  } else {
    write_buffer_ = write_plain_buffer_ = new Buffer;
  }
}

void Connection::Init() {
// do not call shared_from_this() in constructor
#ifdef __linux__
  channel_->SetEpollEdgeTriggered();
#endif
  channel_->UpdateToLoop();
  SetChannelCallbacks();
}

void Connection::SetChannelCallbacks() {
  channel_->set_read_callback(std::bind(&Connection::OnReadCallback, this));
  channel_->set_write_callback(std::bind(&Connection::OnWriteCallback, this));
  channel_->set_close_callback(std::bind(&Connection::OnCloseCallback, this));
  channel_->set_error_callback(std::bind(&Connection::OnCloseCallback, this));
}

Connection::~Connection() {
  socket::close(channel_->fd());
  if (read_buffer_) delete read_buffer_;
  if (write_buffer_) delete write_buffer_;
}

void Connection::Send(const std::string& buf) {
  if (shut_down_) return;
  write_buffer_->Write(buf);

  EnableWrite(WriteBuffer());
}

void Connection::ShutDownWrite() { channel_->ShutDownWrite(); }

void Connection::Close() {
  channel_->ShutDownRead();
  channel_->ShutDownWrite();
}

void Connection::SendFile(std::string&& header, const std::string& filename) {
  if (shut_down_ || !send_file_) return;
  write_file_buffer_->AddFile(std::move(header), filename);

  EnableWrite(WriteBuffer());
}

void Connection::OnReadCallback() {
  if (shut_down_) return;
  int ret = ReadBuffer();
  if (ret == 0) {
    // FIN received
    shut_down_ = true;

    // IMPORTANT: OnCloseCallback can only be called once
    // since the current connection would be destructed after the call
    // OnCloseCallback();

    // IMPORTANT: Cannot return here.
    // Even if ret == 0 there might be data read into buffer
    // return;
  } else if (ret == -1) {
    switch (errno) {
      case ECONNRESET:
        // OnCloseCallback();
        return;
      default:
        break;
    }
  } else if (ret < 0) {
    // error: some negative value other than -1
    shut_down_ = true;
  }

  if (read_callback_ && !read_buffer_->Empty()) {
    read_callback_(shared_from_this(), read_buffer_);
  }

  if (shut_down_ && write_buffer_->Empty()) {
    OnCloseCallback();
  }
}

void Connection::OnWriteCallback() {
  int ret = WriteBuffer();
  if (write_callback_) {
    write_callback_(write_buffer_);
  }
  if (shut_down_ && write_buffer_->Empty()) {
    // channel_->ShutDownWrite();
    OnCloseCallback();
  }

  EnableWrite(ret);
}

void Connection::OnCloseCallback() {
  channel_->ShutDownWrite();
  channel_->RemoveFromLoop();
  if (close_callback_) {
    close_callback_();
  }
}

void Connection::set_read_callback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void Connection::set_write_callback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void Connection::set_close_callback(const ConnectCloseCallback& callback) {
  close_callback_ = callback;
}

ChannelPtr Connection::channel() const { return channel_; }

int Connection::ReadBuffer() {
  return read_buffer_->ReadBufferFromFd(channel_->fd());
}

int Connection::WriteBuffer() {
  return write_buffer_->WriteBufferToFd(channel_->fd());
}

void Connection::EnableWrite(int ret) {
  if (ret < 0) {
    // reached EAGAIN / EWOULDBLOCK,
    // output buffer unavaiable to write
    channel_->EnableWrite();
  } else {
    // output buffer currently available for write
    channel_->EnableWrite(false);
  }
}

}  // namespace ladder
