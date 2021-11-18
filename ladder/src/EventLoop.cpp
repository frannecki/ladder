#include <Channel.h>
#include <EventLoop.h>
#include <Socket.h>
#include <utils.h>

namespace ladder {

#ifdef _MSC_VER
EventLoop::EventLoop(HANDLE iocp_port)
    : iocp_port_(iocp_port), running_(false) {}

void EventLoop::UpdateIocpPort(const Channel* channel) {
  iocp_port_ = ladder::UpdateIocpPort(iocp_port_, channel);
}
#else
EventLoop::EventLoop() : poller_(new EventPoller), running_(false) {}
#endif

void EventLoop::StartLoop() {
  {
    std::lock_guard<std::mutex> lock(mutex_running_);
    if (running_) return;
    running_ = true;
  }
#ifdef _MSC_VER
  DWORD io_size = 0;
  Channel* channel = nullptr;
  LPWSAOVERLAPPED overlapped = nullptr;
#endif
  while (running_) {
#ifdef _MSC_VER
    if (!iocp_port_) continue;
    bool ret =
        GetQueuedCompletionStatus(iocp_port_, &io_size, (PULONG_PTR)&channel,
                                  (LPOVERLAPPED*)&overlapped, INFINITE);
    if (!ret) {
      if (!overlapped) {
        EXIT("GetQueuedCompletionStatus port: %x error: %d",
             iocp_port_, WSAGetLastError());
      } else {
        io_size = -1;
      }
      continue;
    }

    if (channel == nullptr && overlapped == nullptr) {
      continue;
    }

    SocketIocpStatus* status = reinterpret_cast<SocketIocpStatus*>(overlapped);
    channel->HandleEvents(status->status_, io_size);
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

#ifndef _MSC_VER
void EventLoop::UpdateChannel(Channel* channel, int op) {
  poller_->UpdateChannel(channel, op);
}

void EventLoop::RemoveChannel(int fd) { poller_->RemoveChannel(fd); }
#endif

void EventLoop::QueueInLoop(std::function<void()>&& task) {
  pending_tasks_.emplace_back(task);
}

#ifdef __unix__
void EventLoop::set_wakeup_callback(const std::function<void()>& callback) {
  poller_->set_wakeup_callback(callback);
}
#endif

#ifdef __FreeBSD__
int EventLoop::UpdateEvent(const struct kevent* evt) {
  return poller_->UpdateEvent(evt);
}
#endif

}  // namespace ladder
