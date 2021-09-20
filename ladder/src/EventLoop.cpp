#include <EventLoop.h>
#include <Channel.h>

#include <utils.h>

namespace ladder {

EventLoop::EventLoop() : 
  poller_(new EventPoller),
  running_(false)
{

}

void EventLoop::StartLoop() {
  {
    std::lock_guard<std::mutex> lock(mutex_running_);
    running_ = true;
  }
  while(running_) {
    std::vector<ChannelPtr> active_channels;
    poller_->Poll(active_channels);
    for(auto channel : active_channels) {
      channel->HandleEvents();
    }
  }
  std::vector<std::function<void()>> tasks;
  tasks.swap(pending_tasks_);
  for(auto task : tasks) {
    task();
  }
}

void EventLoop::AddChannel(const ChannelPtr& channel) {
  poller_->AddChannel(channel);
}

void EventLoop::RemoveChannel(int fd) {
  poller_->RemoveChannel(fd);
}

void EventLoop::QueueInLoop(std::function<void()>&& task) {
  pending_tasks_.emplace_back(task);
}

void EventLoop::SetWakeupCallback(const std::function<void()>& callback) {
  poller_->SetWakeupCallback(callback);
}

} // namespace ladder
