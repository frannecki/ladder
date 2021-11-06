#ifndef LADDER_CALLBACKS_H
#define LADDER_CALLBACKS_H

#include <functional>
#include <memory>
#include <string>

namespace ladder {

class Connection;
class IBuffer;
class Buffer;
class Channel;
class EventLoop;
class EventLoopThreadPool;
class Acceptor;
class ThreadPool;

using ChannelPtr = std::shared_ptr<Channel>;
using EventLoopPtr = std::shared_ptr<EventLoop>;
using EventLoopThreadPoolPtr = std::shared_ptr<EventLoopThreadPool>;
using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
using AcceptorPtr = std::unique_ptr<Acceptor>;
using ConnectionPtr = std::shared_ptr<Connection>;

using ReadEvtCallback = std::function<void(const ConnectionPtr&, Buffer*)>;
using WriteEvtCallback = std::function<void(IBuffer*)>;
using ConnectCloseCallback = std::function<void()>;
using ConnectionEvtCallback = std::function<void(const ConnectionPtr&)>;

class IBuffer {
 public:
  virtual void Write(const std::string& buf) = 0;
  virtual void Write(const char* src, size_t len) = 0;
  virtual int WriteBufferToFd(int fd) = 0;
  virtual bool Empty() const = 0;
  virtual ~IBuffer();
};

}  // namespace ladder

#endif
