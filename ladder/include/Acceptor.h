#ifndef LADDER_ACCEPTOR_H
#define LADDER_ACCEPTOR_H

#include <memory>
#include <functional>
#include <utils.h>

namespace ladder {

class Channel;
class SocketAddr;
using ChannelPtr = std::shared_ptr<Channel>;
using NewConnectionCallback = std::function<void(int, const SocketAddr&)>;

class Acceptor {
public:
  Acceptor(const ChannelPtr&, bool ipv6);
  void SetNewConnectionCallback(const NewConnectionCallback& callback);

private:
  void HandleAccept();

  ChannelPtr channel_;
  NewConnectionCallback new_connection_callback_;
  bool ipv6_;
};

} // namespace ladder

#endif
