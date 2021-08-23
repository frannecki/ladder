#include <Channel.h>
#include <Connection.h>
#include <EventLoop.h>

namespace ladder {

Connection::Connection(EventLoop* loop, int fd) : 
  channel_(new Channel(loop, fd))
{

}

void Connection::SetReadCallback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
  channel_->SetReadCallback(std::bind(read_callback_, this, read_buffer_));

}

void Connection::SetWriteCallback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
  channel_->SetWriteCallback(std::bind(write_callback_, this, write_buffer_));
}

Channel* Connection::channel() const {
  return channel_;
}

} // namespace ladder
