#ifndef LADDER_CONNECTION_H
#define LADDER_CONNECTION_H

#include <mutex>

#include <Callbacks.h>

namespace ladder
{

class Buffer;
class Channel;
class EventLoop;

using EventLoopPtr = std::shared_ptr<EventLoop>;

class Connection {

public:
  Connection(const EventLoopPtr& loop, int fd);
  ~Connection();
  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  Channel* channel() const;

private:
  std::mutex mutex_;
  Channel* channel_;
  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  Buffer* read_buffer_;
  Buffer* write_buffer_;
};

} // namespace ladder


#endif
