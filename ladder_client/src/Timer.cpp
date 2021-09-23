#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <string.h>

#include <utils.h>
#include <Timer.h>
#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>

#include <Logger.h>

namespace ladder {

Timer::Timer(const EventLoopPtr& loop) :
  loop_(loop) 
{
  timer_fd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if(timer_fd_ < 0) {
    EXIT("[Timer] timerfd_create");
  }
  timer_channel_ = std::make_shared<Channel>(loop, timer_fd_);
  timer_channel_->AddToLoop();
  timer_channel_->SetReadCallback(std::bind(&Timer::OnTimer, this));
}

Timer::~Timer() {
  socket::close(timer_fd_);
}

void Timer::SetTimerEventCallback(const TimerEventCallback& callback) {
  callback_ = callback;
}

void Timer::SetInterval(uint64_t interval, bool periodic) {
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
}

uint64_t Timer::GetInterval() const {
  return interval_;
}

void Timer::OnTimer() {
  uint64_t exp;
  if(::read(timer_fd_, &exp, sizeof(exp)) < 0) {
    EXIT("[Timer] read");
  }
  if(callback_) {
    callback_();
  }
}

} // namespace ladder
