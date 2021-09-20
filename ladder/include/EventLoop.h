#ifndef LADDER_EVENT_LOOP_H
#define LADDER_EVENT_LOOP_H

#include <mutex>
#include <memory>
#include <vector>
#include <functional>

#include <EventPoller.h>

namespace ladder {

class Channel;
using ChannelPtr = std::shared_ptr<Channel>;

class EventPoller;
using EventPollerPtr = std::unique_ptr<EventPoller>;

class EventLoop : public std::enable_shared_from_this<EventLoop> {
public:
  EventLoop();
  void StartLoop();
  void StopLoop();
  void AddChannel(const ChannelPtr& channel);
  void RemoveChannel(int fd);
  void QueueInLoop(std::function<void()>&& task);
  void SetWakeupCallback(const std::function<void()>& callback);
  // TODO: wake up poller for urgent tasks

private:
  EventPollerPtr poller_;
  bool running_;
  std::mutex mutex_running_;
  std::vector<std::function<void()>> pending_tasks_;
};

} // namespace ladder

#endif
