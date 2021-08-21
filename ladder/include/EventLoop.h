#ifndef LADDER_EVENT_LOOP_H
#define LADDER_EVENT_LOOP_H

#include <mutex>
#include <memory>

namespace ladder {

class EventPoller;

class EventLoop {
public:
  EventLoop();
  void StartLoop();

private:
  std::mutex mutex_;
  std::unique_ptr<EventPoller> poller_;
};

} // namespace ladder

#endif
