#include <algorithm>
#include <functional>

#include <EventLoop.h>
#include <EventLoopThreadPool.h>
#include <ThreadPool.h>

namespace ladder {

static const size_t kMinEventLoopThreadNum = 4;

#ifdef _MSC_VER
EventLoopThreadPool::EventLoopThreadPool(HANDLE iocp_port, size_t capacity)
#else
EventLoopThreadPool::EventLoopThreadPool(size_t capacity)
#endif
    : capacity_((std::max)(capacity, kMinEventLoopThreadNum)),
      pool_(new ThreadPool((std::max)(capacity, kMinEventLoopThreadNum))),
      cur_idx_(0) {
  for (size_t i = 0; i < capacity_; ++i) {
#ifdef _MSC_VER
    loops_.emplace_back(std::make_shared<EventLoop>(iocp_port));
#else
    loops_.emplace_back(std::make_shared<EventLoop>());
#endif
  }
  Init();
}

EventLoopThreadPool::~EventLoopThreadPool() { delete pool_; }

void EventLoopThreadPool::Init() {
  for (size_t i = 0; i < capacity_; ++i) {
    pool_->emplace(std::bind(&EventLoop::StartLoop, loops_[i]));
  }
}

EventLoopPtr EventLoopThreadPool::GetNextLoop() {
  std::lock_guard<std::mutex> lock(mutex_cur_idx_);
  if (cur_idx_ == capacity_) {
    cur_idx_ = 0;
  }
  return loops_[cur_idx_++];
}

}  // namespace ladder
