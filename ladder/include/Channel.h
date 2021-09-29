#ifndef LADDER_CHANNEL_H
#define LADDER_CHANNEL_H

#include <stdint.h>
#include <sys/epoll.h>

#include <memory>
#include <functional>

namespace ladder {

class EventLoop;

using EventLoopPtr = std::shared_ptr<EventLoop>;

class Channel {
public:
  Channel(EventLoopPtr loop, int fd);
  ~Channel();
  int fd() const;
  void SetReadCallback(const std::function<void()>& callback);
  void SetWriteCallback(const std::function<void()>& callback);
  void SetCloseCallback(const std::function<void()>& callback);
  void SetErrorCallback(const std::function<void()>& callback);
  void SetEvents(uint32_t events);
  void SetEpollEdgeTriggered(bool edge_triggered = true);
  void EnableWrite(bool enable = true);
  uint32_t events() const;
  uint32_t event_mask() const;
  void HandleEvents();
  void UpdateToLoop(int op = EPOLL_CTL_ADD);
  void RemoveFromLoop();

  void ShutDownWrite();
  void ShutDownRead();
  EventLoopPtr loop() const;

private:
  bool Iswriting() const;
  bool IsReading() const;

  int fd_;
  EventLoopPtr loop_;
  uint32_t events_;
  uint32_t event_mask_;
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
  std::function<void()> close_callback_;
  std::function<void()> error_callback_;
};

} // namespace ladder

#endif
