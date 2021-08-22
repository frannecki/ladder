#ifndef LADDER_EVENT_LOOP_H
#define LADDER_EVENT_LOOP_H

#include <mutex>
#include <memory>

namespace ladder {

class EventPoller;
class Channel;

using ChannelPtr = std::shared_ptr<Channel>;

class EventLoop {
public:
  EventLoop();
  void StartLoop();
  void AddChannel(const ChannelPtr& channel);
  void RemoveChannel(int fd);

private:
  std::unique_ptr<EventPoller> poller_;
  bool running_;
  std::mutex mutex_running_;
};

} // namespace ladder

#endif
