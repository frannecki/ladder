#ifndef LADDER_TCP_SERVER_H
#define LADDER_TCP_SERVER_H

#include <map>
#include <memory>
#include <mutex>

#include <openssl/ssl.h>

#include <Base.h>
#include <Socket.h>

namespace ladder {

class Channel;
class SocketAddr;

class LADDER_API TcpServer {
 public:
  TcpServer(const SocketAddr& addr, bool send_file = false,
            const char* cert_path = nullptr, const char* key_path = nullptr,
            size_t loop_thread_num = 8, size_t working_thread_num = 8);
  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;
  ~TcpServer();
  void Start();
  void set_read_callback(const ReadEvtCallback& callback);
  void set_write_callback(const WriteEvtCallback& callback);
  void set_connection_callback(const ConnectionEvtCallback& callback);
#ifndef _MSC_VER
  EventLoopPtr loop() const;
#endif
  EventLoopThreadPoolPtr loop_threads() const;

 private:
#ifdef _MSC_VER
  void OnNewConnection(int fd, const SocketAddr&, char* buffer, int io_size);
#else
  void OnNewConnectionCallback(int fd, SocketAddr&&);
  void OnNewConnection(int fd, const SocketAddr&);
  ThreadPoolPtr working_threads_;
  size_t working_thread_num_;
#endif
  void OnCloseConnectionCallback(int fd);

  ChannelPtr channel_;
  EventLoopPtr loop_;
  EventLoopThreadPoolPtr loop_threads_;
  AcceptorPtr acceptor_;
  std::map<int, ConnectionPtr> connections_;
  SocketAddr addr_;
  bool send_file_;
  size_t loop_thread_num_;
  std::mutex mutex_connections_;

  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  ConnectionEvtCallback connection_callback_;

#ifdef _MSC_VER
  HANDLE iocp_port_;
#endif

  SSL_CTX* ssl_ctx_;
};

};  // namespace ladder

#endif
