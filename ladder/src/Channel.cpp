#include <Channel.h>

namespace ladder {

Channel::Channel(int fd) :
  fd_(fd),
  events_(0)
{
  
}

void Channel::SetReadCallback(const std::function<void()>& callback) {
  read_callback_ = callback;
}

void Channel::SetWriteCallback(const std::function<void()>& callback) {
  write_callback_ = callback;
}

void Channel::SetEvents(uint32_t events) {
  events_ = events;
}

void HandleEvents() {
  ;
}

int Channel::fd() const {
  return fd_;
}

} // namespace ladder
