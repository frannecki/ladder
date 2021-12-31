#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>
#include <utils.h>

#include <compat.h>

namespace ladder {

#ifdef LADDER_OS_WINDOWS
EventLoop::EventLoop(HANDLE iocp_port)
    : iocp_port_(iocp_port), running_(false) {}

void EventLoop::UpdateIocpPort(const Channel* channel) {
  iocp_port_ = ladder::UpdateIocpPort(iocp_port_, channel);
}

#else
EventLoop::EventLoop() : poller_(new EventPoller), running_(false) {}

void EventLoop::UpdateChannel(Channel* channel, int op) {
  poller_->UpdateChannel(channel, op);
}

void EventLoop::RemoveChannel(int fd) { poller_->RemoveChannel(fd); }
#endif

void EventLoop::StartLoop() {
  {
    std::lock_guard<std::mutex> lock(mutex_running_);
    if (running_) return;
    running_ = true;
  }
#ifdef LADDER_OS_WINDOWS
  DWORD io_size = 0;
  Channel* channel = nullptr;
  SocketIocpStatus* status = nullptr;
#endif
  while (running_) {
#ifdef LADDER_OS_WINDOWS
    if (!iocp_port_) continue;
    bool ret =
        GetQueuedCompletionStatus(iocp_port_, &io_size, (PULONG_PTR)&channel,
                                  (LPOVERLAPPED*)&status, INFINITE);
    
    if (status) {
      status->UpdateRefCount(false);
    }
    
    if (!ret) {
      if (!status) {
        EXIT("GetQueuedCompletionStatus port: %x error: %d",
             iocp_port_, WSAGetLastError());
      }
    }

    if (channel == nullptr && status == nullptr) {
      continue;
    }

    if (status) {
      channel->HandleEvents(status->status_, io_size);
    }
#else
    std::vector<Channel*> active_channels;
    poller_->Poll(active_channels);
    for (auto& channel : active_channels) {
      channel->HandleEvents();
    }
#endif
  }
}

void EventLoop::StopLoop() {
  std::lock_guard<std::mutex> lock(mutex_running_);
  running_ = false;
}

void EventLoop::QueueInLoop(std::function<void()>&& task) {
  pending_tasks_.emplace_back(task);
}

#ifdef LADDER_OS_UNIX
void EventLoop::set_wakeup_callback(const std::function<void()>& callback) {
  poller_->set_wakeup_callback(callback);
}
#endif

#ifdef LADDER_OS_FREEBSD
int EventLoop::UpdateEvent(const struct kevent* evt) {
  return poller_->UpdateEvent(evt);
}
#endif

}  // namespace ladder
