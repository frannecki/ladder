#include <unistd.h>
#ifdef __linux__
#include <sys/timerfd.h>
#elif defined(__FreeBSD__)
#include <sys/event.h>
#include <sys/time.h>
#endif
#include <string.h>

#include <utils.h>
#include <Timer.h>
#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>

namespace ladder {

Timer::Timer(const EventLoopPtr& loop) :
  loop_(loop)
{
#ifdef __linux
	timer_fd_ = ::timerfd_create(CLOCK_MONOTONIC,
															 TFD_NONBLOCK | TFD_CLOEXEC);
	if(timer_fd_ < 0) {
    EXIT("[Timer] timerfd_create");
  }
  timer_channel_ = std::make_shared<Channel>(loop, timer_fd_);
  timer_channel_->UpdateToLoop();
#elif defined(__FreeBSD__)
	timer_fd_ = 1;
  timer_channel_ = std::make_shared<Channel>(loop, timer_fd_);
#endif
  timer_channel_->SetReadCallback(std::bind(&Timer::OnTimer, this));
}

Timer::~Timer() {
#ifdef __linux__
  socket::close(timer_fd_);
#elif defined(__FreeBSD__)
	struct kevent evt;
	EV_SET(&evt, timer_fd_, EVFILT_TIMER,
				 EV_DELETE, 0, 0, NULL);
	loop_->UpdateEvent(&evt);
#endif
}

void Timer::SetTimerEventCallback(const TimerEventCallback& callback) {
  callback_ = callback;
}

void Timer::SetInterval(uint64_t interval, bool periodic) {
#ifdef __linux
  struct itimerspec value;
  bzero(&value, sizeof(value));
  uint64_t microseconds = interval_ = interval;
  uint64_t seconds = microseconds / 1000000;
  uint64_t nanoseconds = (microseconds % 1000000) * 1000;
  value.it_value.tv_sec = seconds;
  value.it_value.tv_nsec = nanoseconds;
  if(periodic) {
    value.it_interval.tv_sec = seconds;
    value.it_interval.tv_nsec = nanoseconds;
  }
  if(::timerfd_settime(timer_fd_, 0, &value, NULL) < 0) {
    EXIT("[Timer] timerfd_settime");
  }
#elif defined(__FreeBSD__)
	struct kevent evt;
	u_short flags = EV_ADD | EV_ENABLE;
	// NOTE: for freebsd there cannot be multiple timers
	// running simultaneously in a single event loop
	EV_SET(&evt, timer_fd_, EVFILT_TIMER,
				 periodic ? flags : (flags | EV_ONESHOT),
				 0, interval / 1000, timer_channel_.get());
	loop_->UpdateEvent(&evt);
#endif
}

uint64_t Timer::GetInterval() const {
  return interval_;
}

void Timer::OnTimer() {
#ifdef __linux__
  uint64_t exp;
  if(socket::read(timer_fd_, &exp, sizeof(exp)) < 0) {
    EXIT("[Timer] read");
  }
#endif
  if(callback_) {
    callback_();
  }
}

} // namespace ladder
