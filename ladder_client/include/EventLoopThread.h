#ifndef LADDER_EVENT_LOOP_THREAD_H
#define LADDER_EVENT_LOOP_THREAD_H

#include <thread>

#include "Base.h"

namespace ladder {

class LADDER_API EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();
  void Stop();
  EventLoopPtr loop() const;

 private:
  EventLoopPtr loop_;
  std::thread thread_;
};

}  // namespace ladder

#endif
