#include <ThreadPool.h>

namespace ladder {

ThreadPool::ThreadPool(size_t capacity) : capacity_(capacity), stopped_(false) {
  Init();
}

void ThreadPool::Init() {
  for (size_t i = 0; i < capacity_; ++i)
    threads_.emplace_back(std::thread([this]() {
      Callback cur_task;
      while (1) {
        {
          std::unique_lock<std::mutex> lock(mutex_);
          condition_.wait(lock,
                          [this]() { return stopped_ || !tasks_.empty(); });
          if (stopped_ && tasks_.empty()) {
            break;
          }
          cur_task = std::move(tasks_.front());
          tasks_.pop();
        }
        cur_task();
      }
    }));
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    stopped_ = true;
  }
  condition_.notify_all();
  for (size_t i = 0; i < threads_.size(); ++i) {
    threads_[i].join();
  }
}

void ThreadPool::emplace(Callback&& f) {
  auto task = std::make_shared<Callback>(f);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    tasks_.emplace([task]() { (*task)(); });
  }
  condition_.notify_one();
}

}  // namespace ladder
