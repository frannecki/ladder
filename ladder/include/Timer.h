#ifndef LADDER_TIMER_H
#define LADDER_TIMER_H

#include <functional>
#include <memory>

namespace ladder {

class Buffer;
class Channel;
class EventLoop;

using ChannelPtr = std::shared_ptr<Channel>;
using TimerEventCallback = std::function<void()>;
using EventLoopPtr = std::shared_ptr<EventLoop>;

class Timer {
public:
  Timer(const EventLoopPtr& loop);
  ~Timer();
  void SetInterval(uint64_t microseconds);
  void SetTimerEventCallback(const TimerEventCallback& callback);
  uint64_t GetInterval() const;
  void OnTimer();

private:
  ChannelPtr timer_channel_;
  EventLoopPtr loop_;
  int timer_fd_;
  TimerEventCallback callback_;
  uint64_t interval_;
};

} // namespace ladder

#endif
