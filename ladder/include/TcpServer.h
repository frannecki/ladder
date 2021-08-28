#ifndef LADDER_TCP_SERVER_H
#define LADDER_TCP_SERVER_H

#include <memory>
#include <map>

#include <utils.h>
#include <Aliases.h>
#include <Socket.h>

namespace ladder {

class Channel;
class SocketAddr;

class TcpServer {
public:
  TcpServer(const SocketAddr& addr);
  ~TcpServer();
  void Start();
  const Channel* channel() const;
  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  void SetConnectionCallback(const ConnectionEvtCallback& callback);
  EventLoopPtr loop() const;

private:
  void OnNewConnectionCallback(int fd, const SocketAddr&);
  void OnCloseConnectionCallback(int fd);

  ChannelPtr channel_;
  EventLoopPtr loop_;
  EventLoopThreadPoolPtr thread_pool_;
  AcceptorPtr acceptor_;
  std::map<int, ConnectionPtr> connections_;
  SocketAddr addr_;

  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  ConnectionEvtCallback connection_callback_;

};

};

#endif
