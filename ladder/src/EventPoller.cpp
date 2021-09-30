#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>

#include <utils.h>
#include <Channel.h>
#include <EventPoller.h>
#include <Socket.h>
#include <MemoryPool.h>

namespace ladder {

const int kEpollTimeoutMs = 10000;

EventPoller::EventPoller() : cur_poll_size_(0) {
  epfd_ = epoll_create1(0);
  if(epfd_ < 0) {
    EXIT("[EventPoller] epoll_create1");
  }
  pipe_.reset(new Pipe);
  UpdateChannel(pipe_->channel().get(), EPOLL_CTL_ADD);
  cur_poll_size_ += 1;
}

EventPoller::~EventPoller() {
  ;
}

void EventPoller::Poll(std::vector<Channel*>& active_channels) {

  bool alloc_success = true;
  struct epoll_event* evts = MemoryPool<epoll_event>::allocate_n(cur_poll_size_);
  if(evts == nullptr) {
    alloc_success = false;
    evts = new struct epoll_event[cur_poll_size_];
  }

  int ret = epoll_wait(epfd_, evts, cur_poll_size_,
                       kEpollTimeoutMs / 10);
  if(ret == -1) {
    switch(errno) {
      case EINTR:
        return;
      default:
        EXIT("[EventPoller] epoll_wait");
    }
  }

  for(int i = 0; i < ret; ++i) {
    uint32_t evt = evts[i].events;
    auto channel = reinterpret_cast<Channel*>(evts[i].data.ptr);
    channel->SetEvents(evt);
    active_channels.emplace_back(channel);
  }

  if(!alloc_success) {
    delete []evts;
  }
  else {
    MemoryPool<epoll_event>::free(evts, true);
  }
}

void EventPoller::UpdateChannel(Channel* channel,
                                int op) {
  struct epoll_event event;
  bzero(&event, sizeof(event));
  event.data.fd = channel->fd();
  event.events = channel->event_mask();
  event.data.ptr = channel;
  int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD,
                      channel->fd(), &event);
  if(ret < 0) {
    switch(errno) {
      case(EEXIST):
        break;
      default:
        EXIT("[EventPoller] epoll_ctl add");
    }
  }
  else {
    cur_poll_size_ += 1;
  }
}

void EventPoller::RemoveChannel(int fd) {
  int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
  if(ret < 0) {
    switch(errno) {
      case(ENOENT):
        break;
      default:
        EXIT("[EventPoller] epoll_ctl del");
    }
  }
  else {
    cur_poll_size_ -= 1;
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
  socket::close(fd_[0]);
  socket::close(fd_[1]);
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
