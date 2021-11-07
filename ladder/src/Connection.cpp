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
#include <utils.h>

namespace ladder {

static const int kMaxWsaBufSize = 1024;

#ifdef _MSC_VER
Connection::Connection(int fd, bool send_file)
    : channel_(new Channel(fd)),
#else
Connection::Connection(const EventLoopPtr& loop, int fd, bool send_file)
    : channel_(new Channel(loop, fd)),
#endif
      read_buffer_(new Buffer),
      write_plain_buffer_(nullptr),
      write_file_buffer_(nullptr),
#ifdef _MSC_VER
      read_wsa_buf_(new WSABUF),
      write_wsa_buf_(new WSABUF),
      read_status_(new SocketIocpStatus(kPollIn)),
      write_status_(new SocketIocpStatus(kPollOut)),
      first_sent_(false),
#endif
      shut_down_(false),
      send_file_(send_file) {
  if (send_file) {
    write_buffer_ = write_file_buffer_ = new FileBuffer;
  } else {
    write_buffer_ = write_plain_buffer_ = new Buffer;
  }
#ifdef _MSC_VER
  read_wsa_buf_->buf = new char[kMaxWsaBufSize + 1];
  write_wsa_buf_->buf = new char[kMaxWsaBufSize + 1];
#endif
}

#ifdef _MSC_VER
void Connection::Init(HANDLE iocp_port, char* buffer, int io_size) {
  SetChannelCallbacks();
  UpdateIocpPort(iocp_port, channel_.get());
#ifdef _MSC_VER
  // read from accepted buffer
  if (io_size > 0) {
    read_buffer_->Write(buffer, io_size);
    if (read_callback_) {
      read_callback_(shared_from_this(), read_buffer_);
    }
  }
  PostRead();
#else
  channel_->UpdateToLoop();
#endif
}
#else
void Connection::Init() {
#ifdef __linux__
  channel_->SetEpollEdgeTriggered();
#endif
  channel_->UpdateToLoop();
  SetChannelCallbacks();
}
#endif

void Connection::SetChannelCallbacks() {
#ifdef _MSC_VER
  channel_->set_read_callback(
    std::bind(&Connection::OnReadCallback, this, std::placeholders::_1));
  channel_->set_write_callback(
    std::bind(&Connection::OnWriteCallback, this, std::placeholders::_1));
#else
  channel_->set_read_callback(std::bind(&Connection::OnReadCallback, this));
  channel_->set_write_callback(std::bind(&Connection::OnWriteCallback, this));
#endif
  channel_->set_close_callback(std::bind(&Connection::OnCloseCallback, this));
  channel_->set_error_callback(std::bind(&Connection::OnCloseCallback, this));
}

Connection::~Connection() {
  socket::close(channel_->fd());
#ifdef _MSC_VER
  delete read_status_;
  delete read_wsa_buf_->buf;
  delete read_wsa_buf_;

  delete write_status_;
  delete write_wsa_buf_->buf;
  delete write_wsa_buf_;
#endif
  if (read_buffer_) delete read_buffer_;
  if (write_buffer_) delete write_buffer_;
}

void Connection::Send(const std::string& buf) {
  if (shut_down_) return;
  write_buffer_->Write(buf);
#ifdef _MSC_VER
  if (!first_sent_) {
    PostWrite();
    first_sent_ = true;
  }
#else
  EnableWrite(WriteBuffer());
#endif
}

void Connection::ShutDownWrite() { channel_->ShutDownWrite(); }

void Connection::Close() {
  channel_->ShutDownRead();
  channel_->ShutDownWrite();
}

void Connection::SendFile(std::string&& header, const std::string& filename) {
  if (shut_down_ || !send_file_) return;
  write_file_buffer_->AddFile(std::move(header), filename);
#ifdef _MSC_VER
#else
  EnableWrite(WriteBuffer());
#endif
}

#ifdef _MSC_VER
void Connection::OnReadCallback(int io_size) {
#else
void Connection::OnReadCallback() {
#endif
  if (shut_down_) return;
#ifdef _MSC_VER
  int ret = ReadBuffer(io_size);
#else
  int ret = ReadBuffer();
#endif
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

#ifdef _MSC_VER
  if (!shut_down_) PostRead();
#endif

  if (shut_down_ && write_buffer_->Empty()) {
    OnCloseCallback();
  }
}

#ifdef _MSC_VER
void Connection::OnWriteCallback(int io_size) {
  int ret = WriteBuffer(io_size);
#else
void Connection::OnWriteCallback() {
  int ret = WriteBuffer();
#endif
  if (write_callback_) {
    write_callback_(write_buffer_);
  }
  if (shut_down_ && write_buffer_->Empty()) {
    // channel_->ShutDownWrite();
    OnCloseCallback();
  }

#ifdef _MSC_VER
  if (!shut_down_) PostWrite();
#else
  EnableWrite(ret);
#endif
}

void Connection::OnCloseCallback() {
  channel_->ShutDownWrite();
#ifndef _MSC_VER
  channel_->RemoveFromLoop();
#endif
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

#ifdef _MSC_VER
int Connection::ReadBuffer(int io_size) {
  if (io_size > 0) {
    read_buffer_->Write(read_wsa_buf_->buf, read_wsa_buf_->len = io_size);
  }
  return io_size;
}

int Connection::WriteBuffer(int io_size) {
  write_buffer_->HaveRead(io_size);
  return io_size;
}

void Connection::PostRead() {
  read_status_->Reset();
  socket::read(channel_->fd(), read_wsa_buf_, read_status_);
}

void Connection::PostWrite() {
  int send_len = write_buffer_->Peek(write_wsa_buf_->buf, kMaxWsaBufSize);
  write_wsa_buf_->len = send_len;
  write_status_->Reset();
  socket::write(channel_->fd(), write_wsa_buf_, write_status_);
}
#else
int Connection::ReadBuffer() {
  return read_buffer_->ReadBufferFromFd(channel_->fd());
}

int Connection::WriteBuffer() {
  return write_buffer_->WriteBufferToFd(channel_->fd());
}
#endif

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
