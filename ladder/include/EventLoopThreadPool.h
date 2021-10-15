#ifndef LADDER_EVENT_LOOP_THREAD_POOL_H
#define LADDER_EVENT_LOOP_THREAD_POOL_H

#include <memory>
#include <mutex>
#include <vector>

namespace ladder {

const size_t kMinEventLoopThreadNum = 4;

class ThreadPool;
class EventLoop;
using EventLoopPtr = std::shared_ptr<EventLoop>;

class EventLoopThreadPool {
 public:
  EventLoopThreadPool(size_t capacity);
  ~EventLoopThreadPool();
  EventLoopPtr GetNextLoop();

 private:
  void Init();

  size_t capacity_;
  ThreadPool* pool_;
  std::vector<EventLoopPtr> loops_;
  size_t cur_idx_;
  std::mutex mutex_cur_idx_;
};

}  // namespace ladder

#endif
