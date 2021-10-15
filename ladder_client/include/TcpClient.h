#ifndef LADDER_TCP_CLIENT_H
#define LADDER_TCP_CLIENT_H

#include <memory>
#include <mutex>

#include <Base.h>
#include <Socket.h>

namespace ladder {

enum TcpConnectionStatus : int {
  kDisconnected = 0,
  kConnecting,
  kConnected,
  kDisConnecting
};

class Connector;
using ConnectorPtr = std::unique_ptr<Connector>;

class TcpClient {
 public:
  TcpClient(const SocketAddr& target_addr, const EventLoopPtr& loop,
            uint16_t retry_initial_timeout, int max_retry = 10);
  ~TcpClient();

  void Connect();
  void Disconnect();

  void SetReadCallback(const ReadEvtCallback& callback);
  void SetWriteCallback(const WriteEvtCallback& callback);
  void SetConnectionCallback(const ConnectionEvtCallback& callback);
  EventLoopPtr loop() const;

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
};

}  // namespace ladder

#endif
