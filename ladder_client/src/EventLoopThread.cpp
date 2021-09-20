#include <EventLoopThread.h>
#include <EventLoop.h>

#include <Logger.h>

namespace ladder {

EventLoopThread::EventLoopThread() : 
  loop_((std::make_shared<EventLoop>()))
{
  thread_ = std::thread(&EventLoop::StartLoop, loop_);
  LOG_DEBUG("EventLoopThread started");
}

EventLoopThread::~EventLoopThread() {
  Stop();
}

void EventLoopThread::Stop() {
  loop_->StopLoop();
  if(thread_.joinable()) {
    thread_.join();
  }
  LOG_DEBUG("EventLoopThread stopped");
}

EventLoopPtr EventLoopThread::loop() const {
  return loop_;
}

} // namespace ladder
