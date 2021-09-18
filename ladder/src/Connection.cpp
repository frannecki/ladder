#include <unistd.h>

#include <functional>

#include <Channel.h>
#include <Connection.h>
#include <EventLoop.h>
#include <Buffer.h>

namespace ladder {

Connection::Connection(const EventLoopPtr& loop, int fd) : 
  channel_(new Channel(loop, fd)),
  read_buffer_(new Buffer),
  write_buffer_(new Buffer)
{

}

void Connection::Init() {
  // do not call shared_from_this() in constructor
  channel_->SetEpollEdgeTriggered();
  channel_->AddToLoop();
  channel_->SetReadCallback(std::bind(&Connection::OnReadCallback, this));
  channel_->SetWriteCallback(std::bind(&Connection::OnWriteCallback, this));
  channel_->SetCloseCallback(std::bind(&Connection::OnCloseCallback, this));
}

Connection::~Connection() {
  ::close(channel_->fd());
  delete read_buffer_;
  delete write_buffer_;
}

void Connection::Send(const std::string& buf) {
  write_buffer_->Write(buf);
  write_buffer_->WriteBufferToFd(channel_->fd());
}

void Connection::OnReadCallback() {
  int ret = read_buffer_->ReadBufferFromFd(channel_->fd());
  if(ret == 0) {
    // FIN accepted
    channel_->ShutDownWrite();
    // IMPORTANT: OnCloseCallback can only be called
    // once since the current connection would be destructed after the call
    // OnCloseCallback();
    return;
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

  if(read_callback_) {
    read_callback_(shared_from_this(), read_buffer_);
  }
}

void Connection::OnWriteCallback() {
  write_buffer_->WriteBufferToFd(channel_->fd());
  if(write_callback_) {
    write_callback_(write_buffer_);
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
  channel_->SetReadCallback(std::bind(read_callback_, shared_from_this(), read_buffer_));
}

void Connection::SetWriteCallback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
  channel_->SetWriteCallback(std::bind(write_callback_, write_buffer_));
}

void Connection::SetCloseCallback(const ConnectCloseCallback& callback) {
  close_callback_ = callback;
}

ChannelPtr Connection::channel() const {
  return channel_;
}

} // namespace ladder
