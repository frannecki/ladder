#include <string.h>
#include <sys/epoll.h>

#include <utils.h>
#include <Channel.h>
#include <EventPoller.h>

namespace ladder {

EventPoller::EventPoller() {
  epfd_ = epoll_create1(0);
  if(epfd_ < 0) {
    EXIT("[EventPoller] epoll_create1");
  }
}

EventPoller::~EventPoller() {
  ;
}

void EventPoller::Poll(std::vector<ChannelPtr>& active_channels) {

  if(channels_.empty()) {
    return;
  }

  int max_evt_num = channels_.size();
  struct epoll_event* evts = new struct epoll_event[max_evt_num];

  int ret = epoll_wait(epfd_, evts, max_evt_num, -1);
  if(ret == -1) {
    switch(errno) {
      case EINTR:
        return;
      default:
        EXIT("[EventPoller] epoll_wait");
    }
  }

  for(int i = 0; i < ret; ++i) {
    int fd = evts[i].data.fd;
    uint32_t evt = evts[i].events;

    ChannelPtr channel;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto iter = channels_.find(fd);
      if(iter == channels_.end() || iter->second == nullptr) {
        continue;
      }
      channel = iter->second;
    }
    channel->SetEvents(evt);
    
    active_channels.emplace_back(std::move(channel));
  }

  delete []evts;
}

void EventPoller::AddChannel(const ChannelPtr& channel) {
  int ret = -1;
  struct epoll_event event;
  bzero(&event, sizeof(event));
  event.data.fd = channel->fd();
  event.events = channel->GetEvents();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    channels_.insert(std::pair<int, ChannelPtr>(
      channel->fd(), channel));
    ret = epoll_ctl(epfd_, EPOLL_CTL_ADD,
                    channel->fd(), &event);
  }
  if(ret < 0) {
    EXIT("[EventPoller] epoll_ctl add");
  }
}

void EventPoller::RemoveChannel(int fd) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = channels_.find(fd);
  if(iter != channels_.end()) {
    iter = channels_.erase(iter);
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
    if(ret < 0) {
      EXIT("[EventPoller] epoll_ctl del");
    }
  }
}

}
