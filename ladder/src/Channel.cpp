#include <sys/epoll.h>

#include <Channel.h>
#include <EventLoop.h>

namespace ladder {

Channel::Channel(EventLoop* loop, int fd) :
  fd_(fd),
  loop_(loop),
  events_(0)
{
  
}

void Channel::SetReadCallback(const std::function<void()>& callback) {
  read_callback_ = callback;
}

void Channel::SetWriteCallback(const std::function<void()>& callback) {
  write_callback_ = callback;
}

void Channel::SetEvents(uint32_t events) {
  events_ = events;
}

void Channel::HandleEvents() {
  if(events_ == EPOLLIN) {
    ;
  }
  else if(events_ == EPOLLOUT) {
    ;
  }
}

int Channel::fd() const {
  return fd_;
}

void Channel::Remove() {
  loop_->RemoveChannel(fd_);
}

} // namespace ladder
