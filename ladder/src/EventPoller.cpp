#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <sys/epoll.h>

#include <utils.h>
#include <Channel.h>
#include <EventPoller.h>

namespace ladder {

const int kEpollTimeoutMs = 10000;

EventPoller::EventPoller() {
  epfd_ = epoll_create1(0);
  if(epfd_ < 0) {
    EXIT("[EventPoller] epoll_create1");
  }
  pipe_.reset(new Pipe);
  AddChannel(pipe_->channel());
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

  int ret = epoll_wait(epfd_, evts, max_evt_num, kEpollTimeoutMs / 10);
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

void EventPoller::Wakeup() {
  pipe_->Wakeup();
}

void EventPoller::SetWakeupCallback(const std::function<void()>& callback) {
  pipe_->SetWakeupCallback(callback);
}

Pipe::Pipe() {
  if(::pipe2(fd_, O_NONBLOCK) < 0) {
    EXIT("pipe2");
  }
  channel_ = std::make_shared<Channel>(nullptr, fd_[0]);
  channel_->SetReadCallback(std::bind(&Pipe::ReadCallback, this));
}

Pipe::~Pipe() {
  ::close(fd_[0]);
  ::close(fd_[1]);
}

void Pipe::Wakeup() {
  char ch;
  ::write(fd_[1], &ch, sizeof(ch));
}

ChannelPtr Pipe::channel() const {
  return channel_;
}

void Pipe::SetWakeupCallback(const std::function<void()>& callback) {
  wakeup_callback_ = callback;
}

void Pipe::ReadCallback() {
  char ch;
  ::read(fd_[0], &ch, sizeof(ch));
  if(wakeup_callback_) {
    wakeup_callback_();
  }
}

}
