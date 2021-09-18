#ifndef LADDER_THREADPOOL_H
#define LADDER_THREADPOOL_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

namespace ladder {

using Callback = std::function<void()>;

class ThreadPool {
public:
  ThreadPool(size_t capacity=10);
  ~ThreadPool();
  void emplace(Callback&& f);

private:
  void Init();
  size_t capacity_;	// total number of threads
  bool stopped_;
  std::queue<Callback> tasks_;
  std::mutex mutex_;
  std::vector<std::thread> threads_;
  std::condition_variable condition_;
};

} // namespace ladder

#endif
