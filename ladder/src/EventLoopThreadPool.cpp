#include <functional>
#include <algorithm>

#include <EventLoopThreadPool.h>
#include <EventLoop.h>
#include <ThreadPool.h>

namespace ladder {

EventLoopThreadPool::EventLoopThreadPool(size_t capacity) : 
  capacity_(std::max(capacity, kMinEventLoopThreadNum)),
  pool_(new ThreadPool(std::max(capacity, kMinEventLoopThreadNum))),
  loops_(std::vector<EventLoopPtr>(std::max(capacity, kMinEventLoopThreadNum),
          std::make_shared<EventLoop>())),
  cur_idx_(0)
{
  Init();
}

EventLoopThreadPool::~EventLoopThreadPool() {
  delete pool_;
}

void EventLoopThreadPool::Init() {
	for(size_t i = 0; i < capacity_; ++i) {
    pool_->emplace(std::bind(&EventLoop::StartLoop, loops_[i]));
  }
}

EventLoopPtr EventLoopThreadPool::GetNextLoop() {
  std::lock_guard<std::mutex> lock(mutex_cur_idx_);
  if(cur_idx_ == capacity_) {
    cur_idx_ = 0;
  }
  return loops_[(cur_idx_++)];
}

} // namespace ladder
