#ifndef LADDER_CHANNEL_H
#define LADDER_CHANNEL_H

#include <stdint.h>

#include "compat.h"

#ifdef LADDER_HAVE_EPOLL
#include <sys/epoll.h>
#elif defined(LADDER_HAVE_KQUEUE)
#include <sys/event.h>
#endif

#include <functional>
#include <memory>

#include "Base.h"

namespace ladder {

class EventLoop;

using EventLoopPtr = std::shared_ptr<EventLoop>;
#ifdef LADDER_OS_WINDOWS
using EventCallback = std::function<void(int)>;
#else
using EventCallback = std::function<void()>;
#endif

class LADDER_API Channel {
 public:
#ifdef LADDER_OS_WINDOWS
  Channel(int fd);
  void HandleEvents(int evts, int io_size);
#else
  Channel(EventLoopPtr loop, int fd);
  void HandleEvents();
  void SetRevents(uint32_t events);
  uint32_t events() const;
  EventLoopPtr loop() const;
  void RemoveFromLoop();
#endif
  ~Channel();
  int fd() const;
  void EnableWrite(bool enable = true);
  void SetReadCallback(const EventCallback& callback);
  void SetWriteCallback(const EventCallback& callback);
  void SetCloseCallback(const std::function<void()>& callback);
  void SetErrorCallback(const std::function<void()>& callback);

#ifdef LADDER_HAVE_EPOLL
  void UpdateToLoop(int op = EPOLL_CTL_ADD);
  void SetEpollEdgeTriggered(bool edge_triggered = true);
#elif defined(LADDER_HAVE_KQUEUE)
  void UpdateToLoop(int op = EV_ADD | EV_ENABLE);  // | EV_CLEAR);
#endif
  void ShutDownWrite();
  void ShutDownRead();

 private:
#ifdef LADDER_OS_WINDOWS
  EventCallback read_callback_;
  EventCallback write_callback_;
  std::function<void()> close_callback_;
  std::function<void()> error_callback_;
#else
  bool IsWriting() const;
  bool IsReading() const;
  EventLoopPtr loop_;
  uint32_t revents_;
  uint32_t events_;

  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
  std::function<void()> close_callback_;
  std::function<void()> error_callback_;
#endif
  int fd_;
};

}  // namespace ladder

#endif
