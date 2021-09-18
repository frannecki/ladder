#include <unistd.h>
#include <sys/epoll.h>

#include <utils.h>
#include <Logger.h>
#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>

namespace ladder {

Channel::Channel(EventLoopPtr loop, int fd) :
  fd_(fd),
  loop_(loop),
  events_(EPOLLIN)
{

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

void Channel::SetEpollEdgeTriggered(bool edge_triggered) {
  if(edge_triggered) {
    events_ |= EPOLLET;
  }
  else {
    events_ &= (~EPOLLET);
  }
}


void Channel::HandleEvents() {
  if(events_ & EPOLLOUT) {
    if(write_callback_) {
      write_callback_();
    }
  }
  if(events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if(read_callback_) {
      read_callback_();
    }
  }
  if((events_ & EPOLLHUP)) {
    if(close_callback_) {
      close_callback_();
    }
  }
  SetEvents(0);
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

void Channel::ShutDownWrite() {
  if(!Iswriting()) {
    socket::shutdown_write(fd_);
  }
}

void Channel::ShutDownRead() {
  if(!IsReading()) {
    socket::shutdown_read(fd_);
  }
}

bool Channel::Iswriting() const {
  return events_ & EPOLLOUT;
}

bool Channel::IsReading() const {
  return events_ & (EPOLLIN | EPOLLPRI);
}

} // namespace ladder
