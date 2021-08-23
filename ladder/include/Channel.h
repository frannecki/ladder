#ifndef LADDER_CHANNEL_H
#define LADDER_CHANNEL_H

#include <stdint.h>

#include <memory>
#include <functional>

namespace ladder {

class EventLoop;

class Channel : public std::enable_shared_from_this<Channel> {
public:
  Channel(EventLoop* loop, int fd);
  ~Channel();
  int fd() const;
  void SetReadCallback(const std::function<void()>& callback);
  void SetWriteCallback(const std::function<void()>& callback);
  void SetCloseCallback(const std::function<void()>& callback);
  void SetEvents(uint32_t events);
  uint32_t GetEvents() const;
  void HandleEvents();
  void AddToLoop();
  void RemoveFromLoop();

private:
  int fd_;
  EventLoop* loop_;
  uint32_t events_;
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
  std::function<void()> close_callback_;
};

} // namespace ladder

#endif
