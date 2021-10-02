#ifndef LADDER_EVENT_POLLER_H
#define LADDER_EVENT_POLLER_H

#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__FreeBSD__)
#include <sys/event.h>
#endif

#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace ladder {

enum kPollEvent : uint32_t {
#ifdef __linux__
  kPollIn = EPOLLIN,
  kPollOut = EPOLLOUT,
  kPollPri = EPOLLPRI,
  kPollRdHup = EPOLLRDHUP,
  kPollHup = EPOLLHUP,
  kPollErr = EPOLLERR,
  kPollEt = EPOLLET,
#elif defined(__FreeBSD__)
  kPollIn = 1 << 0,
  kPollOut = 1 << 2,
  kPollErr = 1 << 3,
#endif
};

class Channel;
using ChannelPtr = std::shared_ptr<Channel>;

class Pipe;
using PipePtr = std::unique_ptr<Pipe>;

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
  void Wakeup();
  void SetWakeupCallback(const std::function<void()>& callback);

private:
  int poll_fd_;
  int cur_poll_size_;
  PipePtr pipe_;
#ifdef __FreeBSD__
  static std::map<short, uint32_t> flt_2_stat_;	// filter to status
  static std::map<uint32_t, short> stat_2_flt_;			// status to filter
#endif
};

class Pipe {
public:
  Pipe();
  ~Pipe();
  void Wakeup();
  ChannelPtr channel() const;
  void SetWakeupCallback(const std::function<void()>& callback);

private:
  void ReadCallback();

  int fd_[2];
  ChannelPtr channel_;
  std::function<void()> wakeup_callback_;
};

} // namespace ladder

#endif
