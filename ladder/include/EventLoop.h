#ifndef LADDER_EVENT_LOOP_H
#define LADDER_EVENT_LOOP_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

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
  void UpdateChannel(Channel* channel, int op);
  void RemoveChannel(int fd);
  void QueueInLoop(std::function<void()>&& task);
#ifdef __unix__
  void set_wakeup_callback(const std::function<void()>& callback);
#endif
  // TODO: wake up poller for urgent tasks
#ifdef __FreeBSD__
  int UpdateEvent(const struct kevent* evt);
#endif

 private:
  EventPollerPtr poller_;
  bool running_;
  std::mutex mutex_running_;
  std::vector<std::function<void()>> pending_tasks_;
};

}  // namespace ladder

#endif
