#ifndef LADDER_TIMER_H
#define LADDER_TIMER_H

#include <functional>


namespace ladder {

class Buffer;
class Channel;

using ReadEvtCallback = std::function<void(Buffer*)>;

class Timer {
public:
  Timer();
  void SetInterval(int milliseconds);
  void SetReadCallback(const ReadEvtCallback& callback);

private:
  Channel* timer_channel_;
};

} // namespace ladder

#endif
