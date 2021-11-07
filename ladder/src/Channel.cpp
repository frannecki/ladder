#ifdef __unix__
#include <unistd.h>
#endif
#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <Channel.h>
#include <EventLoop.h>
#include <Logging.h>
#include <Socket.h>

namespace ladder {

#ifdef _MSC_VER
Channel::Channel(int fd) :
#else
Channel::Channel(EventLoopPtr loop, int fd) :
    loop_(loop),
    revents_(0),
// kPollOut set initially
#ifdef __linux__
    events_(kPollEvent::kPollIn | kPollEvent::kPollRdHup |
            kPollEvent::kPollOut),
#else
    events_(kPollEvent::kPollIn),
#endif
#endif
    fd_(fd)
{
}

Channel::~Channel() { LOGF_DEBUG("Destroying channel fd = %d", fd_); }

void Channel::set_read_callback(const EventCallback& callback) {
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

#ifndef _MSC_VER
void Channel::set_revents(uint32_t events) { revents_ |= events; }

uint32_t Channel::events() const { return events_; }
#endif

#ifdef __linux__
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
#endif

void Channel::EnableWrite(bool enable) {
#ifdef __linux__
  if (enable) {
    events_ |= kPollEvent::kPollOut;
  } else {
    events_ &= (~kPollEvent::kPollOut);
  }
  UpdateToLoop(EPOLL_CTL_MOD);
#elif defined(__FreeBSD__)
  events_ = kPollEvent::kPollOut;
  // UpdateToLoop(enable ? (EV_ADD | EV_ENABLE | EV_CLEAR) : EV_DISABLE);
  UpdateToLoop(enable ? (EV_ADD | EV_ENABLE) : EV_DISABLE);
#endif
}

#ifdef _MSC_VER
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
void Channel::HandleEvents() {
  uint32_t evts = revents_;
  revents_ = 0;
#ifdef __linux__
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
#ifdef __linux__
  if (evts &
      (kPollEvent::kPollIn | kPollEvent::kPollRdHup)) {
#else
  if (evts & kPollEvent::kPollIn) {
#endif
    if (read_callback_) {
      read_callback_();
    }
  }
}
#endif

int Channel::fd() const { return fd_; }

void Channel::ShutDownWrite() {
#ifndef _MSC_VER
  if (!IsWriting())
#endif
    socket::shutdown_write(fd_);
}

void Channel::ShutDownRead() {
#ifndef _MSC_VER
  if (!IsReading())
#endif
    socket::shutdown_read(fd_);
}

#ifndef _MSC_VER

void Channel::UpdateToLoop(int op) { loop_->UpdateChannel(this, op); }

void Channel::RemoveFromLoop() {
  if (loop_) {
    loop_->RemoveChannel(fd_);
  }
}

bool Channel::IsWriting() const { return events_ & kPollEvent::kPollOut; }

bool Channel::IsReading() const {
#ifdef __linux__
  return events_ & (kPollEvent::kPollIn | kPollEvent::kPollPri);
#else
  return events_ & kPollEvent::kPollIn;
#endif
}

EventLoopPtr Channel::loop() const { return loop_; }
#endif

}  // namespace ladder
