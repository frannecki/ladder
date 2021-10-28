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

namespace ladder {

class EventLoop;

using EventLoopPtr = std::shared_ptr<EventLoop>;

class Channel {
 public:
  Channel(EventLoopPtr loop, int fd);
  ~Channel();
  int fd() const;
  void set_read_callback(const std::function<void()>& callback);
  void set_write_callback(const std::function<void()>& callback);
  void set_close_callback(const std::function<void()>& callback);
  void set_error_callback(const std::function<void()>& callback);
  void set_revents(uint32_t events);
  void EnableWrite(bool enable = true);
  uint32_t events() const;
  void HandleEvents();
#ifdef __linux__
  void UpdateToLoop(int op = EPOLL_CTL_ADD);
  void SetEpollEdgeTriggered(bool edge_triggered = true);
#elif defined(__FreeBSD__)
  void UpdateToLoop(int op = EV_ADD | EV_ENABLE);  // | EV_CLEAR);
#endif
  void UpdateToLoop(int op = 0);
  void RemoveFromLoop();

  void ShutDownWrite();
  void ShutDownRead();
  EventLoopPtr loop() const;

 private:
  bool Iswriting() const;
  bool IsReading() const;

  int fd_;
  EventLoopPtr loop_;
  uint32_t revents_;
  uint32_t events_;
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
  std::function<void()> close_callback_;
  std::function<void()> error_callback_;
};

}  // namespace ladder

#endif
