#ifndef LADDER_EVENT_LOOP_H
#define LADDER_EVENT_LOOP_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <EventPoller.h>
#endif

#include <Base.h>

namespace ladder {

class EventPoller;
using EventPollerPtr = std::unique_ptr<EventPoller>;

class LADDER_API EventLoop {
 public:
#ifdef _MSC_VER
  EventLoop(HANDLE iocp_port = nullptr);
  void UpdateIocpPort(const Channel* channel);
#else
  EventLoop();
  void UpdateChannel(Channel* channel, int op);
  void RemoveChannel(int fd);
#endif
  void StartLoop();
  void StopLoop();
  void QueueInLoop(std::function<void()>&& task);
#ifdef __unix__
  void set_wakeup_callback(const std::function<void()>& callback);
#endif
  // TODO: wake up poller for urgent tasks
#ifdef __FreeBSD__
  int UpdateEvent(const struct kevent* evt);
#endif

 private:
#ifdef _MSC_VER
  HANDLE iocp_port_;
#else
  EventPollerPtr poller_;
#endif
  bool running_;
  std::mutex mutex_running_;
  std::vector<std::function<void()>> pending_tasks_;
};

}  // namespace ladder

#endif
