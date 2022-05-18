#ifndef LADDER_BASE_H
#define LADDER_BASE_H

#include <functional>
#include <memory>
#include <string>

#include "compat.h"

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
using EventLoopThreadPoolPtr = std::unique_ptr<EventLoopThreadPool>;
using ThreadPoolPtr = std::unique_ptr<ThreadPool>;
using AcceptorPtr = std::unique_ptr<Acceptor>;
using ConnectionPtr = std::shared_ptr<Connection>;

using ReadEvtCallback = std::function<void(const ConnectionPtr&, Buffer*)>;
using WriteEvtCallback = std::function<void(IBuffer*)>;
using ConnectCloseCallback = std::function<void()>;
using ConnectionEvtCallback = std::function<void(const ConnectionPtr&)>;

class LADDER_API IBuffer {
 public:
  virtual ~IBuffer();
  virtual void Write(const std::string& buf) = 0;
  virtual void Write(const char* src, size_t len) = 0;
#ifndef LADDER_OS_WINDOWS
  virtual int WriteBufferToFd(int fd) = 0;
#endif
  virtual uint32_t Peek(char* dst, size_t len) = 0;
  virtual bool Empty() const = 0;
  virtual void HaveRead(size_t) = 0;
};

}  // namespace ladder

#endif
