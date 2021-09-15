#ifndef LADDER_CALLBACKS_H
#define LADDER_CALLBACKS_H

#include <functional>
#include <memory>

namespace ladder {

class Connection;
class Buffer;
class Channel;
class EventLoop;
class EventLoopThreadPool;
class Acceptor;

using ChannelPtr = std::shared_ptr<Channel>;
using EventLoopPtr = std::shared_ptr<EventLoop>;
using EventLoopThreadPoolPtr = std::shared_ptr<EventLoopThreadPool>;
using AcceptorPtr = std::unique_ptr<Acceptor>;
using ConnectionPtr = std::shared_ptr<Connection>;

using ReadEvtCallback = std::function<void(const ConnectionPtr&, Buffer*)>;
using WriteEvtCallback = std::function<void(Buffer*)>;
using ConnectCloseCallback = std::function<void()>;
using ConnectionCloseCallbackPtr = std::unique_ptr<ConnectCloseCallback>;
using ConnectionEvtCallback = std::function<void(const ConnectionPtr&)>;

} // namespace ladder

#endif
