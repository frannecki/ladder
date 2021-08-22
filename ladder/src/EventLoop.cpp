#include <EventLoop.h>
#include <Channel.h>
#include <EventPoller.h>

namespace ladder {

EventLoop::EventLoop() : 
  running_(false)
{
  ;
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
}

void EventLoop::AddChannel(const ChannelPtr& channel) {
  poller_->AddChannel(channel);
}

void EventLoop::RemoveChannel(int fd) {
  poller_->RemoveChannel(fd);
}

} // namespace ladder
