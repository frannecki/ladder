#ifdef __unix__
#include <unistd.h>
#endif
#ifdef __linux__
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

#ifdef __linux__
static thread_local epoll_event poll_evts[kPollLimit];
#elif defined(__FreeBSD__)
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

#endif

EventPoller::EventPoller() : cur_poll_size_(0) {
  pipe_.reset(new Pipe);
#ifdef __linux__
  poll_fd_ = epoll_create1(0);
  if (poll_fd_ < 0) {
    EXIT("[EventPoller] epoll_create1");
  }
  UpdateChannel(pipe_->channel(), EPOLL_CTL_ADD);
#elif defined(__FreeBSD__)
  poll_fd_ = kqueue();
  if (poll_fd_ < 0) {
    EXIT("[EventPoller] kqueue");
  }
  UpdateChannel(pipe_->channel(), EV_ADD);
#endif
  cur_poll_size_ += 1;
}

EventPoller::~EventPoller() {}

void EventPoller::Poll(std::vector<Channel*>& active_channels) {
  int ret = 0;
#ifdef __linux__
  memset(poll_evts, 0, kPollLimit * sizeof(epoll_event));
  ret = epoll_wait(poll_fd_, poll_evts, kPollLimit,
                       kPollTimeoutMs / 10);
  if (ret == -1) {
    switch (errno) {
      case EINTR:
        return;
      default:
        EXIT("[EventPoller] epoll_wait");
    }
  }
#elif defined(__FreeBSD__)
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
#endif

  for (int i = 0; i < ret; ++i) {
    uint32_t evt = 0;
#ifdef __linux__
    evt = poll_evts[i].events;
    Channel* channel = reinterpret_cast<Channel*>(poll_evts[i].data.ptr);
#elif defined(__FreeBSD__)
    short flt = poll_evts[i].filter;
    Channel* channel = reinterpret_cast<Channel*>(poll_evts[i].udata);
    if (poll_evts[i].flags & EV_ERROR) {
      channel->set_revents(kPollEvent::kPollErr);
      continue;
    }

    auto iter = flt_2_stat_.find(flt);
    if (iter != flt_2_stat_.end()) {
      evt = iter->second;
    }
#else
    Channel* channel = nullptr;
#endif
    channel->set_revents(evt);
    active_channels.emplace_back(channel);
  }
}

void EventPoller::UpdateChannel(Channel* channel, int op) {
#ifdef __linux__
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
  }
#elif defined(__FreeBSD__)
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
  if (ret < 0) {
    // kevent failed
  }
#else
  if (1) {
  }
#endif
  else {
    cur_poll_size_ += 1;
  }
}

#ifdef __FreeBSD__
int EventPoller::UpdateEvent(const struct kevent* evt) {
  return kevent(poll_fd_, evt, 1, NULL, 0, NULL);
}
#endif

void EventPoller::RemoveChannel(int fd) {
#ifdef __linux__
  int ret = epoll_ctl(poll_fd_, EPOLL_CTL_DEL, fd, NULL);
  if (ret < 0) {
    switch (errno) {
      case ENOENT:
        break;
      default:
        EXIT("[EventPoller] epoll_ctl del");
    }
  }
#elif defined(__FreeBSD__)
  int ret = -1;
  for (int idx = 0; idx < kNumValidFilters; ++idx) {
    int n = kqueue_ctl(poll_fd_, fd, valid_filters[idx], EV_DELETE, NULL);
    if (n > 0) {
      ret = n;
    }
  }
  if (ret < 0) {
    // no event deleted
  }
#else
  if (1) {
  }
#endif
  else {
    cur_poll_size_ -= 1;
  }
}

void EventPoller::Wakeup() { pipe_->Wakeup(); }

void EventPoller::set_wakeup_callback(const std::function<void()>& callback) {
  pipe_->set_wakeup_callback(callback);
}

#ifdef __FreeBSD__

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

Pipe::Pipe() {
#ifdef __unix__
  if (::pipe2(fd_, O_NONBLOCK) < 0) {
    EXIT("pipe2");
  }
  channel_ = new Channel(nullptr, fd_[0]);
  channel_->set_read_callback(std::bind(&Pipe::ReadCallback, this));
}

Pipe::~Pipe() {
  socket::close(fd_[0]);
  socket::close(fd_[1]);
  if (channel_) delete channel_;
}

void Pipe::Wakeup() {
  char ch;
  socket::write(fd_[1], &ch, sizeof(ch));
}

Channel* Pipe::channel() const { return channel_; }

void Pipe::set_wakeup_callback(const std::function<void()>& callback) {
  wakeup_callback_ = callback;
}

void Pipe::ReadCallback() {
  char ch;
  socket::read(fd_[0], &ch, sizeof(ch));
  if (wakeup_callback_) {
    wakeup_callback_();
  }
}

}  // namespace ladder
