#ifndef LADDER_CONNECTOR_H
#define LADDER_CONNECTOR_H

#include <memory>

#include <Base.h>
#include <Socket.h>

namespace ladder {

using ConnectionCallback = std::function<void(SocketAddr&&)>;
using ConnectionFailureCallback = std::function<void()>;

class Timer;
using TimerPtr = std::unique_ptr<Timer>;

const uint16_t kMinRetryInitialTimeout = 5;

class Connector {
 public:
#ifdef LADDER_OS_WINDOWS
  Connector(const ChannelPtr&, int max_retry, const SocketAddr& addr,
            const SocketAddr& local_addr,
            uint16_t retry_initial_timeout = kMinRetryInitialTimeout * 2);
#else
  Connector(const ChannelPtr&, int max_retry, const SocketAddr& addr,
            uint16_t retry_initial_timeout = kMinRetryInitialTimeout * 2);
#endif
  Connector(const Connector&) = delete;
  Connector& operator=(const Connector&) = delete;
  ~Connector();
  void set_connection_callback(const ConnectionCallback& callback);
  void set_connection_failure_callback(const ConnectionFailureCallback& callback);
  void Start();

 private:
  void HandleConnect();
  void Retry();

  ChannelPtr channel_;
  int retry_;
  int max_retry_;
  uint16_t retry_timeout_;
  TimerPtr timer_;
  ConnectionCallback connection_callback_;
  ConnectionFailureCallback connection_failure_callback_;
  bool ipv6_;
  SocketAddr addr_;
#ifdef LADDER_OS_WINDOWS
  SocketIocpStatus* status_;
  SocketAddr local_addr_;
#endif
};

}  // namespace ladder

#endif
