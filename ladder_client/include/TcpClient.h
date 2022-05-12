#ifndef LADDER_TCP_CLIENT_H
#define LADDER_TCP_CLIENT_H

#include <memory>
#include <mutex>

#include "Base.h"
#include "Socket.h"

namespace ladder {

enum TcpConnectionStatus : int {
  kDisconnected = 0,
  kConnecting,
  kConnected,
  kDisConnecting
};

class Connector;
using ConnectorPtr = std::unique_ptr<Connector>;

class LADDER_API TcpClient {
 public:
  TcpClient(const SocketAddr& target_addr, const EventLoopPtr& loop,
            uint16_t retry_initial_timeout, bool use_ssl = false,
            int max_retry = 10);
  ~TcpClient();

#ifdef LADDER_OS_WINDOWS
  void Connect(const SocketAddr& local_addr);
#else
  void Connect();
  EventLoopPtr loop() const;
#endif
  void Disconnect();

  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  void SetConnectionCallback(const ConnectionEvtCallback& callback);

 private:
  void OnConnectionCallback(SocketAddr&&);
  void OnConnectionFailureCallback();
  void OnCloseConnectionCallback(int fd);

  int max_retry_;
  int status_;
  std::mutex mutex_status_;
  SocketAddr target_addr_;
  ConnectionPtr conn_;
  ConnectorPtr connector_;
  EventLoopPtr loop_;
  uint16_t retry_initial_timeout_;

  ReadEvtCallback read_callback_;
  WriteEvtCallback write_callback_;
  ConnectionEvtCallback connection_callback_;

  SSL_CTX* ssl_ctx_;
};

}  // namespace ladder

#endif
