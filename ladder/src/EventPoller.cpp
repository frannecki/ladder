#include <compat.h>
#ifndef LADDER_OS_WINDOWS
#ifdef LADDER_OS_UNIX
#include <unistd.h>
#endif
#ifdef LADDER_OS_LINUX
#include <sys/epoll.h>
#endif
#include <fcntl.h>
#include <string.h>

#include <Channel.h>
#include <EventPoller.h>
#include <MemoryPool.h>
#include <Socket.h>
#include <utils.h>

namespace ladder {

static const int kPollLimit = 64;
static const int kPollTimeoutMs = 10000;

//// For Linux
#ifdef LADDER_OS_LINUX
static thread_local epoll_event poll_evts[kPollLimit];

EventPoller::EventPoller() : cur_poll_size_(0) {
  pipe_.reset(new Pipe);
  poll_fd_ = epoll_create1(0);
  if (poll_fd_ < 0) {
    EXIT("[EventPoller] epoll_create1");
  }
  UpdateChannel(pipe_->channel(), EPOLL_CTL_ADD);
  cur_poll_size_ += 1;
}

void EventPoller::Poll(std::vector<Channel*>& active_channels) {
  int ret = 0;
  memset(poll_evts, 0, kPollLimit * sizeof(epoll_event));
  ret = epoll_wait(poll_fd_, poll_evts, kPollLimit, kPollTimeoutMs / 10);
  if (ret == -1) {
    switch (errno) {
      case EINTR:
        return;
      default:
        EXIT("[EventPoller] epoll_wait");
    }
  }

  for (int i = 0; i < ret; ++i) {
    uint32_t evt = 0;
    evt = poll_evts[i].events;
    Channel* channel = reinterpret_cast<Channel*>(poll_evts[i].data.ptr);
    channel->SetRevents(evt);
    active_channels.emplace_back(channel);
  }
}

void EventPoller::UpdateChannel(Channel* channel, int op) {
  struct epoll_event event;
  bzero(&event, sizeof(event));
  event.data.fd = channel->fd();
  event.events = channel->events();
  event.data.ptr = channel;
  int ret = epoll_ctl(poll_fd_, op, channel->fd(), &event);
  if (ret < 0) {
    switch (errno) {
      case EEXIST:
        break;
      default:
        EXIT("[EventPoller] epoll_ctl op = %d", op);
    }
  } else {
    cur_poll_size_ += 1;
  }
}

void EventPoller::RemoveChannel(int fd) {
  int ret = epoll_ctl(poll_fd_, EPOLL_CTL_DEL, fd, NULL);
  if (ret < 0) {
    switch (errno) {
      case ENOENT:
        break;
      default:
        EXIT("[EventPoller] epoll_ctl del");
    }
  } else {
    cur_poll_size_ -= 1;
  }
}

//// For FreeBSD
#elif defined(LADDER_OS_FREEBSD)
static thread_local struct kevent poll_evts[kPollLimit];

const int kNumValidFilters = 3;
short valid_filters[] = {EVFILT_READ, EVFILT_WRITE, EVFILT_TIMER};

static int kqueue_ctl(int kq, uintptr_t ident, short filter, u_short flags,
                      void* udata) {
  struct kevent evt;
  EV_SET(&evt, ident, filter, flags, 0, 0, udata);
  int ret = kevent(kq, &evt, 1, NULL, 0, NULL);
  if (ret < 0) {
    switch (errno) {
      case EBADF:
      case ENOENT:
      case EINTR:
        break;
      default:
        EXIT("[EventPoller] kevent del");
    }
  }
  return ret;
}

EventPoller::EventPoller() : cur_poll_size_(0) {
  pipe_.reset(new Pipe);
  poll_fd_ = kqueue();
  if (poll_fd_ < 0) {
    EXIT("[EventPoller] kqueue");
  }
  UpdateChannel(pipe_->channel(), EV_ADD);
  cur_poll_size_ += 1;
}

void EventPoller::Poll(std::vector<Channel*>& active_channels) {
  int ret = 0;
  // TODO: set kevent timeout
  memset(poll_evts, 0, kPollLimit * sizeof(struct kevent));
  ret = kevent(poll_fd_, NULL, 0, poll_evts, kPollLimit, NULL);
  if (ret == -1) {
    switch (errno) {
      case EINTR:
        return;
      default:
        EXIT("[EventPoller] kevent poll");
    }
  }

  for (int i = 0; i < ret; ++i) {
    uint32_t evt = 0;
    short flt = poll_evts[i].filter;
    Channel* channel = reinterpret_cast<Channel*>(poll_evts[i].udata);
    if (poll_evts[i].flags & EV_ERROR) {
      channel->SetRevents(kPollEvent::kPollErr);
      continue;
    }

    auto iter = flt_2_stat_.find(flt);
    if (iter != flt_2_stat_.end()) {
      evt = iter->second;
    }
    channel->SetRevents(evt);
    active_channels.emplace_back(channel);
  }
}

void EventPoller::UpdateChannel(Channel* channel, int op) {
  int ret = -1;
  uint32_t event_mask = channel->events();
  while (event_mask) {
    uint32_t status = event_mask & (-event_mask);
    event_mask ^= status;
    auto iter = stat_2_flt_.find(status);
    if (iter == stat_2_flt_.end()) {
      continue;
    }
    int n = kqueue_ctl(poll_fd_, channel->fd(), iter->second, op, channel);
    if (n > 0) {
      ret = n;
    }
  }
  if (ret >= 0) {
    // kevent failed
    cur_poll_size_ += 1;
  }
}

void EventPoller::RemoveChannel(int fd) {
  int ret = -1;
  for (int idx = 0; idx < kNumValidFilters; ++idx) {
    int n = kqueue_ctl(poll_fd_, fd, valid_filters[idx], EV_DELETE, NULL);
    if (n > 0) {
      ret = n;
    }
  }
  if (ret >= 0) {
    // no event deleted
    cur_poll_size_ -= 1;
  }
}

int EventPoller::UpdateEvent(const struct kevent* evt) {
  return kevent(poll_fd_, evt, 1, NULL, 0, NULL);
}

// map between filters and status

std::unordered_map<short, uint32_t> EventPoller::flt_2_stat_ = {
    {EVFILT_READ, kPollEvent::kPollIn},
    {EVFILT_WRITE, kPollEvent::kPollOut},
    {EVFILT_TIMER, kPollEvent::kPollIn},
    {EVFILT_USER, kPollEvent::kPollErr}  // use user event to indicate error
};

std::unordered_map<uint32_t, short> EventPoller::stat_2_flt_ = {
    {kPollEvent::kPollIn, EVFILT_READ},
    {kPollEvent::kPollOut, EVFILT_WRITE},
    {kPollEvent::kPollErr, EVFILT_USER}};

#endif

EventPoller::~EventPoller() {}

void EventPoller::Wakeup() { pipe_->Wakeup(); }

void EventPoller::SetWakeupCallback(const std::function<void()>& callback) {
  pipe_->SetWakeupCallback(callback);
}


#ifdef LADDER_OS_UNIX
Pipe::Pipe() {
  if (::pipe2(fd_, O_NONBLOCK) < 0) {
    EXIT("pipe2");
  }
  channel_ = new Channel(nullptr, fd_[0]);
  channel_->SetReadCallback(std::bind(&Pipe::ReadCallback, this));
}

Pipe::~Pipe() {
  socket::close(fd_[0]);
  socket::close(fd_[1]);
  if (channel_) delete channel_;
}

void Pipe::Wakeup() {
  char ch = 0;
  socket::write(fd_[1], &ch, sizeof(ch));
}

Channel* Pipe::channel() const { return channel_; }

void Pipe::SetWakeupCallback(const std::function<void()>& callback) {
  wakeup_callback_ = callback;
}

void Pipe::ReadCallback() {
  char ch;
  socket::read(fd_[0], &ch, sizeof(ch));
  if (wakeup_callback_) {
    wakeup_callback_();
  }
}
#endif

}  // namespace ladder

#endif
