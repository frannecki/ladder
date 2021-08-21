#ifndef LADDER_EVENT_POLLER_H
#define LADDER_EVENT_POLLER_H

#include <map>
#include <vector>
#include <memory>

namespace ladder {

class Channel;
using ChannelPtr = std::shared_ptr<Channel>;

class EventPoller {
public:
  EventPoller();
  ~EventPoller();
  void Poll(std::vector<ChannelPtr>& active_channels);
  void AddChannel(const ChannelPtr& channel);
  void RemoveChannel(int fd);

private:
  int epfd_;
  std::map<int, ChannelPtr> channels_;
};

} // namespace ladder

#endif
