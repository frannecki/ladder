#include <unistd.h>
#include <sys/epoll.h>

#include <utils.h>
#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>

namespace ladder {

Channel::Channel(EventLoopPtr loop, int fd) :
  fd_(fd),
  loop_(loop),
  // EPOLLOUT set initially
  event_mask_(EPOLLIN | EPOLLRDHUP | EPOLLOUT)
{

}

Channel::~Channel() {

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

void Channel::SetErrorCallback(const std::function<void()>& callback) {
  error_callback_ = callback;
}

void Channel::SetEvents(uint32_t events) {
  events_ = events;
}

uint32_t Channel::events() const {
  return events_;
}

uint32_t Channel::event_mask() const {
  return event_mask_;
}

void Channel::SetEpollEdgeTriggered(bool edge_triggered) {
  // IMPORTANT: re-consider whether to add EPOLLOUT here.
  // Since the socket is writable most of the time, almost
  // every trigger will set this flag.
  if(edge_triggered) {
    event_mask_ |= EPOLLET;
  }
  else {
    event_mask_ &= (~EPOLLET);
  }
}

void Channel::EnableWrite(bool enable) {
  if(enable) {
    event_mask_ |= EPOLLOUT;
  }
  else {
    event_mask_ &= (~EPOLLOUT);
  }
  UpdateToLoop(EPOLL_CTL_MOD);
}


void Channel::HandleEvents() {
  if(events_ & EPOLLHUP) {
    // normally EPOLLHUP is not generated.
    // peer close signal (EPOLLRDHUP) is mostly handled after read or write
    if(close_callback_) {
      close_callback_();
    }
    return;
  }
  if(events_ & EPOLLERR) {
    if(error_callback_) {
      error_callback_();
    }
    return;
  }
  if(events_ & EPOLLOUT) {
    if(write_callback_) {
      write_callback_();
    }
  }
  // Pay close attention to the order of event handing.
  // Handling read event could cause this channel to deconstruct
  if(events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if(read_callback_) {
      read_callback_();
    }
  }
}

int Channel::fd() const {
  return fd_;
}

void Channel::UpdateToLoop(int op) {
  loop_->UpdateChannel(this, op);
}

void Channel::RemoveFromLoop() {
  if(loop_) {
    loop_->RemoveChannel(fd_);
  }
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

EventLoopPtr Channel::loop() const {
  return loop_;
}

} // namespace ladder
