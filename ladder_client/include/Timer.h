#ifndef LADDER_TIMER_H
#define LADDER_TIMER_H

#include <functional>
#include <memory>

#include <compat.h>

namespace ladder {

class Buffer;
class Channel;
class EventLoop;

using ChannelPtr = std::shared_ptr<Channel>;
using TimerEventCallback = std::function<void()>;
using EventLoopPtr = std::shared_ptr<EventLoop>;

class Timer {
 public:
#ifdef LADDER_OS_WINDOWS
  Timer();
#else
  Timer(const EventLoopPtr& loop);
#endif
  ~Timer();
  void SetInterval(uint64_t microseconds, bool periodic = false);
  void SetTimmerEventCallback(const TimerEventCallback& callback);
  uint64_t GetInterval() const;
  void OnTimer();

 private:
  ChannelPtr timer_channel_;
#ifdef LADDER_OS_WINDOWS
  HANDLE timer_;
  HANDLE timer_queue_;
#else
  EventLoopPtr loop_;
  int timer_fd_;
#endif
  TimerEventCallback callback_;
  uint64_t interval_;
};

}  // namespace ladder

#endif
