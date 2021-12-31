#ifndef LADDER_EVENT_LOOP_THREAD_POOL_H
#define LADDER_EVENT_LOOP_THREAD_POOL_H

#include <memory>
#include <mutex>
#include <vector>

#include <compat.h>

namespace ladder {

class ThreadPool;
class EventLoop;
using EventLoopPtr = std::shared_ptr<EventLoop>;

class LADDER_API EventLoopThreadPool {
 public:
#ifdef LADDER_OS_WINDOWS
  EventLoopThreadPool(HANDLE iocp_port, size_t capacity);
#else
  EventLoopThreadPool(size_t capacity);
#endif
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
