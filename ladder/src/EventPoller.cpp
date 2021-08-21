#include <sys/epoll.h>

#include <Channel.h>
#include <EventPoller.h>

namespace ladder {

EventPoller::EventPoller() {
  epfd_ = epoll_create(0);
  if(epfd_ < 0) {
    perror("[EventPoller] epoll_create");
    exit(-1);
  }
}

EventPoller::~EventPoller() {
  ;
}

void EventPoller::Poll(std::vector<ChannelPtr>& active_channels) {
  int max_evt_num = 1 + channels_.size();
  struct epoll_event* evts = new struct epoll_event[max_evt_num];
  int ret = epoll_wait(epfd_, evts, max_evt_num, -1);
  if(ret == -1) {
    perror("[EventPoller] epoll_wait");
    exit(-1);
  }
  for(size_t i = 0; i < ret; ++i) {
    int fd = evts[i].data.fd;
    uint32_t evt = evts[i].events;
    auto iter = channels_.find(fd);
    if(iter != channels_.end()) {
      iter->second->SetEvents(evt);
      active_channels.push_back(iter->second);
    }
  }
}

void EventPoller::AddChannel(const ChannelPtr& channel) {
  channels_.insert(std::pair<int, ChannelPtr>(channel->fd(), channel));
  int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, channel->fd(), NULL);
  if(ret < 0) {
    perror("[EventPoller] epoll_ctl add");
    exit(-1);
  }
}

void EventPoller::RemoveChannel(int fd) {
  auto iter = channels_.find(fd);
  if(iter != channels_.end()) {
    iter = channels_.erase(iter);
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
    if(ret < 0) {
      perror("[EventPoller] epoll_ctl del");
      exit(-1);
    }
  }
}

}
