#ifndef LADDER_EVENT_POLLER_H
#define LADDER_EVENT_POLLER_H

#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace ladder {

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
  void RemoveChannel(int fd);
  void Wakeup();
  void SetWakeupCallback(const std::function<void()>& callback);

private:
  int epfd_;
  int cur_poll_size_;
  PipePtr pipe_;
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
