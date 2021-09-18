#ifndef LADDER_TCP_SERVER_H
#define LADDER_TCP_SERVER_H

#include <memory>
#include <map>
#include <mutex>

#include <utils.h>
#include <Aliases.h>
#include <Socket.h>

namespace ladder {

class Channel;
class SocketAddr;

class TcpServer {
public:
  TcpServer(const SocketAddr& addr,
            size_t loop_thread_num=8,
            size_t working_thread_num=8);
  ~TcpServer();
  void Start();
  const Channel* channel() const;
  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  void SetConnectionCallback(const ConnectionEvtCallback& callback);
  EventLoopPtr loop() const;
  EventLoopThreadPoolPtr loop_threads() const;

private:
  void OnNewConnectionCallback(int fd, SocketAddr&&);
  void OnNewConnection(int fd, const SocketAddr&);
  void OnCloseConnectionCallback(int fd);

  ChannelPtr channel_;
  EventLoopPtr loop_;
  EventLoopThreadPoolPtr loop_threads_;
  ThreadPoolPtr working_threads_;
  AcceptorPtr acceptor_;
  std::map<int, ConnectionPtr> connections_;
  SocketAddr addr_;
  size_t loop_thread_num_;
  size_t working_thread_num_;
  std::mutex mutex_connections_;

  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  ConnectionEvtCallback connection_callback_;

};

};

#endif
