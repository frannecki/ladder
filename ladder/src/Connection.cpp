#include <unistd.h>

#include <functional>

#include <Channel.h>
#include <Connection.h>
#include <EventLoop.h>
#include <Buffer.h>

namespace ladder {

Connection::Connection(const EventLoopPtr& loop, int fd) : 
  channel_(new Channel(loop, fd))
{
  channel_->AddToLoop();
}

void Connection::Init() {
  // do not call shared_from_this() in constructor
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
}

void Connection::OnReadCallback() {
  if(read_buffer_->ReadBufferFromFd(channel_->fd()) == 0) {
    // FIN accepted
    // channel_->ShutDownWrite();
  }
  else {
    if(read_callback_) {
      read_callback_(shared_from_this(), read_buffer_);
    }
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
  close_callback_();
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

Channel* Connection::channel() const {
  return channel_;
}

} // namespace ladder
