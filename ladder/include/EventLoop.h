#ifndef LADDER_EVENT_LOOP_H
#define LADDER_EVENT_LOOP_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "compat.h"

#ifdef LADDER_OS_WINDOWS
#include <winsock2.h>
#else
#include "EventPoller.h"
#endif

#include "Base.h"

namespace ladder {

class EventPoller;
using EventPollerPtr = std::unique_ptr<EventPoller>;

class LADDER_API EventLoop {
 public:
#ifdef LADDER_OS_WINDOWS
  EventLoop(HANDLE iocp_port = nullptr);
  void UpdateIocpPort(const Channel* channel);
#else
  EventLoop();
  void UpdateChannel(Channel* channel, int op);
  void RemoveChannel(int fd);
  void Wakeup();
#endif
  ~EventLoop();
  void StartLoop();
  void StopLoop();
  void QueueInLoop(std::function<void()>&& task);
#ifdef LADDER_OS_UNIX
  void SetWakeupCallback(const std::function<void()>& callback);
#endif
  // TODO: wake up poller for urgent tasks
#ifdef LADDER_HAVE_KQUEUE
  int UpdateEvent(const struct kevent* evt);
#endif

 private:
#ifdef LADDER_OS_WINDOWS
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
