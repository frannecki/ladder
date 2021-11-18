#ifndef LADDER_CHANNEL_H
#define LADDER_CHANNEL_H

#include <stdint.h>
#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__FreeBSD__)
#include <sys/event.h>
#endif

#include <functional>
#include <memory>

#include <Base.h>

namespace ladder {

class EventLoop;

using EventLoopPtr = std::shared_ptr<EventLoop>;
#ifdef _MSC_VER
using EventCallback = std::function<void(int)>;
#else
using EventCallback = std::function<void()>;
#endif

class LADDER_API Channel {
 public:
#ifdef _MSC_VER
  Channel(int fd);
#else
  Channel(EventLoopPtr loop, int fd);
#endif
  ~Channel();
  int fd() const;
  void EnableWrite(bool enable = true);
  void set_read_callback(const EventCallback& callback);
  void set_write_callback(const EventCallback& callback);
  void set_close_callback(const std::function<void()>& callback);
  void set_error_callback(const std::function<void()>& callback);
#ifdef _MSC_VER
  void HandleEvents(int evts, int io_size);
#else
  void HandleEvents();
  void set_revents(uint32_t events);
  uint32_t events() const;
#endif
#ifdef __linux__
  void UpdateToLoop(int op = EPOLL_CTL_ADD);
  void SetEpollEdgeTriggered(bool edge_triggered = true);
#elif defined(__FreeBSD__)
  void UpdateToLoop(int op = EV_ADD | EV_ENABLE);  // | EV_CLEAR);
#endif
  void ShutDownWrite();
  void ShutDownRead();
#ifndef _MSC_VER
  EventLoopPtr loop() const;
  void UpdateToLoop(int op = 0);
  void RemoveFromLoop();
#endif

 private:
  int fd_;
#ifndef _MSC_VER
  EventLo bool IsWriting() const;
  bool IsReading() const;
  opPtr loop_;
  uint32_t revents_;
  uint32_t events_;

  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
  std::function<void()> close_callback_;
  std::function<void()> error_callback_;
#endif
  EventCallback read_callback_;
  EventCallback write_callback_;
  std::function<void()> close_callback_;
  std::function<void()> error_callback_;
};

}  // namespace ladder

#endif
