#include <unistd.h>

#include <utils.h>
#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>

#include <Logging.h>

namespace ladder {

Channel::Channel(EventLoopPtr loop, int fd) :
  fd_(fd),
  loop_(loop),
  events_(0),
  // POLL_OUT set initially
#ifdef __linux__
  event_mask_(kPollEvent::kPollIn | kPollEvent::kPollRdHup | kPollEvent::kPollOut)
#elif defined(__FreeBSD__)
  event_mask_(kPollEvent::kPollIn)
#endif
{

}

Channel::~Channel() {
  LOGF_DEBUG("Destroying channel fd = %d", fd_);
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
	events_ |= events;
}

uint32_t Channel::events() const {
  return events_;
}

uint32_t Channel::event_mask() const {
  return event_mask_;
}

#ifdef __linux__
void Channel::SetEpollEdgeTriggered(bool edge_triggered) {
  // IMPORTANT: re-consider whether to add POLL_OUT here.
  // Since the socket is writable most of the time, almost
  // every trigger will set this flag.
  if(edge_triggered) {
    event_mask_ |= kPollEvent::kPollEt;
  }
  else {
    event_mask_ &= (~kPollEvent::kPollEt);
  }
}
#endif

void Channel::EnableWrite(bool enable) {
#ifdef __linux__
  if(enable) {
    event_mask_ |= kPollEvent::kPollOut;
  }
  else {
    event_mask_ &= (~kPollEvent::kPollOut);
  }
  UpdateToLoop(EPOLL_CTL_MOD);
#elif defined(__FreeBSD__)
  event_mask_ = kPollEvent::kPollOut;
  // UpdateToLoop(enable ? (EV_ADD | EV_ENABLE | EV_CLEAR) : EV_DISABLE);
  UpdateToLoop(enable ? (EV_ADD | EV_ENABLE) : EV_DISABLE);
#endif
}


void Channel::HandleEvents() {
#ifdef __linux__
  if(events_ & kPollEvent::kPollHup) {
    // normally POLL_HUP is not generated.
    // peer close signal (POLL_RDHUP) is 
    // mostly handled after read or write
    if(close_callback_) {
      close_callback_();
    }
    return;
  }
#endif
  if(events_ & kPollEvent::kPollErr) {
    if(error_callback_) {
      error_callback_();
    }
    return;
  }
  if(events_ & kPollEvent::kPollOut) {
    if(write_callback_) {
      write_callback_();
    }
  }
  // Pay close attention to the order of event handing.
  // Handling read event could cause this channel to deconstruct
#ifdef __linux__
  if(events_ & (kPollEvent::kPollIn | kPollEvent::kPollOut | kPollEvent::kPollRdHup)) {
#elif defined(__FreeBSD__)
  if(events_ & kPollEvent::kPollIn) {
#endif
    if(read_callback_) {
      read_callback_();
    }
  }
  events_ = 0;
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
  return events_ & kPollEvent::kPollOut;
}

bool Channel::IsReading() const {
#ifdef __linux__
  return events_ & (kPollEvent::kPollIn | kPollEvent::kPollPri);
#elif defined(__FreeBSD__)
  return events_ & kPollEvent::kPollIn;
#endif
}

EventLoopPtr Channel::loop() const {
  return loop_;
}


} // namespace ladder

