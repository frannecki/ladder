#include <compat.h>
#ifdef LADDER_OS_UNIX
#include <unistd.h>
#endif
#ifdef LADDER_OS_LINUX
#include <sys/epoll.h>
#endif

#include <Channel.h>
#include <EventLoop.h>
#include <Logging.h>
#include <Socket.h>

namespace ladder {

#ifdef LADDER_OS_WINDOWS
Channel::Channel(int fd)
    :
#else
Channel::Channel(EventLoopPtr loop, int fd)
    : loop_(loop),
      revents_(0),
// kPollOut set initially
#ifdef LADDER_OS_LINUX
      events_(kPollEvent::kPollIn | kPollEvent::kPollRdHup |
              kPollEvent::kPollOut),
#else
      events_(kPollEvent::kPollIn),
#endif
#endif
      fd_(fd) {
}

Channel::~Channel() { LOGF_DEBUG("Destroying channel fd = %d", fd_); }

void Channel::SetReadCallback(const EventCallback& callback) {
  read_callback_ = callback;
}

void Channel::set_write_callback(const EventCallback& callback) {
  write_callback_ = callback;
}

void Channel::set_close_callback(const std::function<void()>& callback) {
  close_callback_ = callback;
}

void Channel::set_error_callback(const std::function<void()>& callback) {
  error_callback_ = callback;
}

#ifdef LADDER_OS_WINDOWS

void Channel::HandleEvents(int evts, int io_size) {
  if (evts & kPollEvent::kPollOut) {
    if (write_callback_) {
      write_callback_(io_size);
    }
  }
  if (evts & kPollEvent::kPollIn) {
    if (read_callback_) {
      read_callback_(io_size);
    }
  }
}

#else

void Channel::set_revents(uint32_t events) { revents_ |= events; }

uint32_t Channel::events() const { return events_; }

void Channel::HandleEvents() {
  uint32_t evts = revents_;
  revents_ = 0;
#ifdef LADDER_OS_LINUX
  if (evts & kPollEvent::kPollHup) {
    // normally POLL_HUP is not generated.
    // peer close signal (POLL_RDHUP) is
    // mostly handled after read or write
    if (close_callback_) {
      close_callback_();
    }
    return;
  }
#endif
  if (evts & kPollEvent::kPollErr) {
    if (error_callback_) {
      error_callback_();
    }
    return;
  }
  if (evts & kPollEvent::kPollOut) {
    if (write_callback_) {
      write_callback_();
    }
  }
    // Pay close attention to the order of event handing.
    // Handling read event could cause this channel to deconstruct
#ifdef LADDER_OS_LINUX
  if (evts & (kPollEvent::kPollIn | kPollEvent::kPollRdHup)) {
#else
  if (evts & kPollEvent::kPollIn) {
#endif
    if (read_callback_) {
      read_callback_();
    }
  }
}

void Channel::UpdateToLoop(int op) { loop_->UpdateChannel(this, op); }

void Channel::RemoveFromLoop() {
  if (loop_) {
    loop_->RemoveChannel(fd_);
  }
}

bool Channel::IsWriting() const { return events_ & kPollEvent::kPollOut; }

bool Channel::IsReading() const {
#ifdef LADDER_OS_LINUX
  return events_ & (kPollEvent::kPollIn | kPollEvent::kPollPri);
#else
  return events_ & kPollEvent::kPollIn;
#endif
}

EventLoopPtr Channel::loop() const { return loop_; }
#endif

#ifdef LADDER_OS_LINUX
void Channel::SetEpollEdgeTriggered(bool edge_triggered) {
  // IMPORTANT: re-consider whether to add POLL_OUT here.
  // Since the socket is writable most of the time, almost
  // every trigger will set this flag.
  if (edge_triggered) {
    events_ |= kPollEvent::kPollEt;
  } else {
    events_ &= (~kPollEvent::kPollEt);
  }
}

void Channel::EnableWrite(bool enable) {
  if (enable) {
    events_ |= kPollEvent::kPollOut;
  } else {
    events_ &= (~kPollEvent::kPollOut);
  }
  UpdateToLoop(EPOLL_CTL_MOD);
}
#elif defined(LADDER_OS_FREEBSD)
void Channel::EnableWrite(bool enable) {
  events_ = kPollEvent::kPollOut;
  UpdateToLoop(enable ? (EV_ADD | EV_ENABLE) : EV_DISABLE);
}
#else
void Channel::EnableWrite(bool enable) {}
#endif

int Channel::fd() const { return fd_; }

void Channel::ShutDownWrite() {
#ifndef LADDER_OS_WINDOWS
  if (!IsWriting())
#endif
    socket::shutdown_write(fd_);
}

void Channel::ShutDownRead() {
#ifndef LADDER_OS_WINDOWS
  if (!IsReading())
#endif
    socket::shutdown_read(fd_);
}

}  // namespace ladder
