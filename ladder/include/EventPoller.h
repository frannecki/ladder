#ifndef LADDER_EVENT_POLLER_H
#define LADDER_EVENT_POLLER_H

#ifndef _MSC_VER

#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__FreeBSD__)
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

#ifdef __unix__
class Pipe {
 public:
  Pipe();
  ~Pipe();
  void Wakeup();
  Channel* channel() const;
  void set_wakeup_callback(const std::function<void()>& callback);

 private:
  void ReadCallback();

  int fd_[2];
  Channel* channel_;
  std::function<void()> wakeup_callback_;
};
using PipePtr = std::unique_ptr<Pipe>;
#endif

class EventPoller {
 public:
  EventPoller();
  ~EventPoller();
  void Poll(std::vector<Channel*>& active_channels);
  void UpdateChannel(Channel* channel, int op);
#ifdef __FreeBSD__
  int UpdateEvent(const struct kevent* evt);
#endif
  void RemoveChannel(int fd);
#ifdef __unix__
  void Wakeup();
  void set_wakeup_callback(const std::function<void()>& callback);
#endif

 private:
  int poll_fd_;
  int cur_poll_size_;
#ifdef __unix__
  PipePtr pipe_;
#endif
#ifdef __FreeBSD__
  static std::unordered_map<short, uint32_t> flt_2_stat_;  // filter to status
  static std::unordered_map<uint32_t, short> stat_2_flt_;  // status to filter
#endif
};

}  // namespace ladder

#endif

#endif
