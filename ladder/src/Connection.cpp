#include <unistd.h>

#include <functional>

#include <Channel.h>
#include <Connection.h>
#include <EventLoop.h>
#include <Buffer.h>

#include <Socket.h>

namespace ladder {

Connection::Connection(const EventLoopPtr& loop, int fd) : 
  channel_(new Channel(loop, fd)),
  read_buffer_(new Buffer),
  write_buffer_(new Buffer),
  shut_down_(false)
{

}

void Connection::Init() {
  // do not call shared_from_this() in constructor
#ifdef __linux
  channel_->SetEpollEdgeTriggered();
#endif
  channel_->UpdateToLoop();
  SetChannelCallbacks();
}

void Connection::SetChannelCallbacks() {
  channel_->SetReadCallback(std::bind(&Connection::OnReadCallback, this));
  channel_->SetWriteCallback(std::bind(&Connection::OnWriteCallback, this));
  channel_->SetCloseCallback(std::bind(&Connection::OnCloseCallback, this));
  channel_->SetErrorCallback(std::bind(&Connection::OnCloseCallback, this));
}

Connection::~Connection() {
  socket::close(channel_->fd());
  delete read_buffer_;
  delete write_buffer_;
}

void Connection::Send(const std::string& buf) {
  if(shut_down_) {
    return;
  }
  write_buffer_->Write(buf);
  write_buffer_->WriteBufferToFd(channel_->fd());
}

void Connection::OnReadCallback() {
  if(shut_down_)  return;
  int ret = read_buffer_->ReadBufferFromFd(channel_->fd());
  if(ret == 0) {
    // FIN received
    shut_down_ = true;

    // IMPORTANT: OnCloseCallback can only be called once
    // since the current connection would be destructed after the call
    // OnCloseCallback();
    
    // IMPORTANT: Cannot return here.
    // Even if ret == 0 there might be data read into buffer
    // return;
  }
  else if(ret == -1) {
    switch(errno) {
      case ECONNRESET:
        // OnCloseCallback();
        return;
      default:
        break;
    }
  }

  if(read_callback_ && !read_buffer_->Empty()) {
    read_callback_(shared_from_this(), read_buffer_);
  }

  if(shut_down_ && write_buffer_->Empty()) {
    OnCloseCallback();
  }
}

void Connection::OnWriteCallback() {
  int ret = write_buffer_->WriteBufferToFd(channel_->fd());
  if(write_callback_) {
    write_callback_(write_buffer_);
  }
  if(shut_down_ && write_buffer_->Empty()) {
    // channel_->ShutDownWrite();
    OnCloseCallback();
  }
  
  if(ret < 0) {
    // reached EAGAIN / EWOULDBLOCK,
    // output buffer unavaiable to write
    channel_->EnableWrite();
  }
  else {
    // output buffer currently available for write
    channel_->EnableWrite(false);
  }
}

void Connection::OnCloseCallback() {
  channel_->ShutDownWrite();
  channel_->RemoveFromLoop();
  if(close_callback_) {
    close_callback_();
  }
}

void Connection::SetReadCallback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void Connection::SetWriteCallback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void Connection::SetCloseCallback(const ConnectCloseCallback& callback) {
  close_callback_ = callback;
}

ChannelPtr Connection::channel() const {
  return channel_;
}

} // namespace ladder
