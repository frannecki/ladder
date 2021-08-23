#include <unistd.h>
#include <sys/epoll.h>

#include <Logger.h>
#include <Channel.h>
#include <EventLoop.h>

namespace ladder {

Channel::Channel(EventLoop* loop, int fd) :
  fd_(fd),
  loop_(loop),
  events_(EPOLLIN)
{
  ;
}

Channel::~Channel() {
  RemoveFromLoop();
}

void Channel::SetReadCallback(const std::function<void()>& callback) {
  read_callback_ = callback;
}

void Channel::SetWriteCallback(const std::function<void()>& callback) {
  write_callback_ = callback;
}

void Channel::SetCloseCallback(const std::function<void()>& callback) {
  close_callback_ = callback;
}

void Channel::SetEvents(uint32_t events) {
  events_ = events;
}

uint32_t Channel::GetEvents() const {
  return events_;
}

void Channel::HandleEvents() {
  if(events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if(read_callback_) {
      read_callback_();
    }
  }
  else if(events_ & EPOLLOUT) {
    if(write_callback_) {
      write_callback_();
    }
  }
  else if((events_ & EPOLLHUP) && !(events_ & EPOLLIN)) {
    if(close_callback_) {
      close_callback_();
      LOG_DEBUG("close_callback_()");
    }
  }
}

int Channel::fd() const {
  return fd_;
}

void Channel::AddToLoop() {
  loop_->AddChannel(shared_from_this());
}

void Channel::RemoveFromLoop() {
  loop_->RemoveChannel(fd_);
}

} // namespace ladder
