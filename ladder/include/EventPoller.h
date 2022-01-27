#ifndef LADDER_EVENT_POLLER_H
#define LADDER_EVENT_POLLER_H

#include <compat.h>
#ifndef LADDER_OS_WINDOWS

#ifdef LADDER_OS_LINUX
#include <sys/epoll.h>
#elif defined(LADDER_OS_FREEBSD)
#include <sys/event.h>
#include <unordered_map>
#endif

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace ladder {

class Channel;

class Pipe {
 public:
  Pipe();
  ~Pipe();
  void Wakeup();
  Channel* channel() const;
  void SetWakeupCallback(const std::function<void()>& callback);

 private:
  void ReadCallback();

  int fd_[2];
  Channel* channel_;
  std::function<void()> wakeup_callback_;
};
using PipePtr = std::unique_ptr<Pipe>;

class EventPoller {
 public:
  EventPoller();
  ~EventPoller();
  void Poll(std::vector<Channel*>& active_channels);
  void UpdateChannel(Channel* channel, int op);
#ifdef LADDER_OS_FREEBSD
  int UpdateEvent(const struct kevent* evt);
#endif
  void RemoveChannel(int fd);
#ifdef LADDER_OS_UNIX
  void Wakeup();
  void SetWakeupCallback(const std::function<void()>& callback);
#endif

 private:
  int poll_fd_;
  int cur_poll_size_;
#ifdef LADDER_OS_UNIX
  PipePtr pipe_;
#endif
#ifdef LADDER_OS_FREEBSD
  static std::unordered_map<short, uint32_t> flt_2_stat_;  // filter to status
  static std::unordered_map<uint32_t, short> stat_2_flt_;  // status to filter
#endif
};

}  // namespace ladder

#endif

#endif
