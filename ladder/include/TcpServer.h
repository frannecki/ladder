#ifndef LADDER_TCP_SERVER_H
#define LADDER_TCP_SERVER_H

#include <memory>
#include <map>

#include <utils.h>
#include <Callbacks.h>

namespace ladder {

class Channel;
class EventLoop;
class EventLoopThreadPool;
class Acceptor;
class Connection;
class Buffer;

using ChannelPtr = std::shared_ptr<Channel>;
using EventLoopPtr = std::shared_ptr<EventLoop>;
using EventLoopThreadPoolPtr = std::shared_ptr<EventLoopThreadPool>;
using AcceptorPtr = std::unique_ptr<Acceptor>;
using ConnectionPtr = std::shared_ptr<Connection>;

class TcpServer {
public:
  TcpServer(const SocketAddr& addr, bool ipv6 = true);
  ~TcpServer();
  void Start();
  const Channel* channel() const;
  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  EventLoopPtr loop() const;

private:
  void OnNewConnectionCallback(int fd);

  ChannelPtr channel_;
  EventLoopPtr loop_;
  EventLoopThreadPoolPtr pool_;
  AcceptorPtr acceptor_;
  std::map<int, ConnectionPtr> connections_;
  bool ipv6_;

  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;

};

};

#endif
