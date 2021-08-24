#include <string.h>
#include <sys/epoll.h>

#include <utils.h>
#include <Channel.h>
#include <EventPoller.h>
#include <Logger.h>

namespace ladder {

static const int kEpollWaitTimeout = 10000; // 10ms

EventPoller::EventPoller() {
  epfd_ = epoll_create1(0);
  if(epfd_ < 0) {
    perror("[EventPoller] epoll_create1");
    exit(-1);
  }
}

EventPoller::~EventPoller() {
  ;
}

void EventPoller::Poll(std::vector<ChannelPtr>& active_channels) {
  int max_evt_num = channels_.size();
  struct epoll_event* evts = new struct epoll_event[max_evt_num];

  std::lock_guard<std::mutex> lock(mutex_);

  int ret = epoll_wait(epfd_, evts, max_evt_num,
                       kEpollWaitTimeout / 1000);
  if(ret == -1) {
    perror("[EventPoller] epoll_wait");
    exit(-1);
  }
  for(int i = 0; i < ret; ++i) {
    int fd = evts[i].data.fd;
    uint32_t evt = evts[i].events;
    auto iter = channels_.find(fd);
    if(iter != channels_.end()) {
      iter->second->SetEvents(evt);
      active_channels.push_back(iter->second);
    }
  }

  delete []evts;
}

void EventPoller::AddChannel(const ChannelPtr& channel) {
  int ret = -1;
  struct epoll_event event;
  bzero(&event, sizeof(event));
  event.data.fd = channel->fd();
  // using edge trigger
  event.events = channel->GetEvents() | EPOLLET;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    channels_.insert(std::pair<int, ChannelPtr>(
      channel->fd(), channel));
    ret = epoll_ctl(epfd_, EPOLL_CTL_ADD,
                    channel->fd(), &event);
  }
  if(ret < 0) {
    exit_fatal("[EventPoller] epoll_ctl add");
  }
}

void EventPoller::RemoveChannel(int fd) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto iter = channels_.find(fd);
  if(iter != channels_.end()) {
    iter = channels_.erase(iter);
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
    if(ret < 0) {
      exit_fatal("[EventPoller] epoll_ctl del");
    }
  }
}

}
