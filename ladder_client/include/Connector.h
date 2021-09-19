#ifndef LADDER_TCP_CONNECTOR_H
#define LADDER_TCP_CONNECTOR_H

#include <Aliases.h>
#include <Socket.h>

namespace ladder {

using ConnectionCallback = std::function<void(SocketAddr&&)>;
using ConnectionFailureCallback = std::function<void()>;

class Connector {

public:
  Connector(const ChannelPtr&, int max_retry, bool ipv6);
  void SetConnectionCallback(const ConnectionCallback& callback);
  void SetConnectionFailureCallback(const ConnectionFailureCallback& callback);
  void Start(const SocketAddr& addr);

private:
  void HandleConnect();
  void Retry();

  int max_retry_;
  int retry_;
  ChannelPtr channel_;
  ConnectionCallback connection_callback_;
  ConnectionFailureCallback connection_failure_callback_;
  bool ipv6_;

};

} // namespace ladder

#endif
