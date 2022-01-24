#include <compat.h>
#ifdef LADDER_OS_UNIX
#include <unistd.h>
#endif
#ifdef LADDER_OS_LINUX
#include <sys/timerfd.h>
#elif defined(LADDER_OS_FREEBSD)
#include <sys/event.h>
#include <sys/time.h>
#endif
#include <string.h>

#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>
#include <utils.h>

#include <Timer.h>

namespace ladder {

#ifdef LADDER_OS_WINDOWS
static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
  (*(TimerEventCallback*)lpParam)();
}
#endif

#ifdef LADDER_OS_WINDOWS
Timer::Timer()
    : timer_(NULL),
      timer_queue_(CreateTimerQueue()),
      interval_(0) {
  if (!timer_queue_) {
    EXIT("CreateTimerQueue error: %d", GetLastError());
  }
}

Timer::~Timer() {
  CloseHandle(timer_);
  CloseHandle(timer_queue_);
}

void Timer::set_interval(uint64_t interval, bool periodic) {
  uint64_t milliseconds = interval / 1000;
  if (!timer_) {
    if (!CreateTimerQueueTimer(&timer_, timer_queue_,
                               (WAITORTIMERCALLBACK)TimerRoutine, &callback_,
                                milliseconds, periodic ? milliseconds : 0, 0)) {
      EXIT("CreateTimerQueueTimer error: %d", GetLastError());
    }
  } else {
    if (!ChangeTimerQueueTimer(timer_queue_, timer_,
                               milliseconds, periodic ? milliseconds : 0)) {
      EXIT("ChangeTimerQueueTimer error: %d", GetLastError());
    }
  }
}

#else

Timer::Timer(const EventLoopPtr& loop)
    : loop_(loop),
      interval_(0) {
#ifdef LADDER_OS_LINUX
  timer_fd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd_ < 0) {
    EXIT("[Timer] timerfd_create");
  }
  timer_channel_ = std::make_shared<Channel>(loop, timer_fd_);
  timer_channel_->UpdateToLoop();
  timer_channel_->SetReadCallback(std::bind(&Timer::OnTimer, this));
#elif defined(LADDER_OS_FREEBSD)
  timer_fd_ = 1;
  timer_channel_ = std::make_shared<Channel>(loop, timer_fd_);
  timer_channel_->SetReadCallback(std::bind(&Timer::OnTimer, this));
#endif
}

Timer::~Timer() {
#ifdef LADDER_OS_LINUX
  socket::close(timer_fd_);
#elif defined(LADDER_OS_FREEBSD)
  struct kevent evt;
  EV_SET(&evt, timer_fd_, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
  loop_->UpdateEvent(&evt);
#endif
}

void Timer::set_interval(uint64_t interval, bool periodic) {
#ifdef LADDER_OS_LINUX
  struct itimerspec value;
  bzero(&value, sizeof(value));
  uint64_t microseconds = interval_ = interval;
  uint64_t seconds = microseconds / 1000000;
  uint64_t nanoseconds = (microseconds % 1000000) * 1000;
  value.it_value.tv_sec = seconds;
  value.it_value.tv_nsec = nanoseconds;
  if (periodic) {
    value.it_interval.tv_sec = seconds;
    value.it_interval.tv_nsec = nanoseconds;
  }
  if (::timerfd_settime(timer_fd_, 0, &value, NULL) < 0) {
    EXIT("[Timer] timerfd_settime");
  }
#elif defined(LADDER_OS_FREEBSD)
  struct kevent evt;
  u_short flags = EV_ADD | EV_ENABLE;
  // NOTE: for freebsd there cannot be multiple timers
  // running simultaneously in a single event loop
  EV_SET(&evt, timer_fd_, EVFILT_TIMER, periodic ? flags : (flags | EV_ONESHOT),
         0, interval / 1000, timer_channel_.get());
  loop_->UpdateEvent(&evt);
#endif
}

#endif

void Timer::set_timer_event_callback(const TimerEventCallback& callback) {
  callback_ = callback;
}

uint64_t Timer::GetInterval() const { return interval_; }

void Timer::OnTimer() {
#ifdef LADDER_OS_LINUX
  uint64_t exp;
  if (socket::read(timer_fd_, &exp, sizeof(exp)) < 0) {
    EXIT("[Timer] read");
  }
#endif
  if (callback_) {
    callback_();
  }
}

}  // namespace ladder
