#ifndef LADDER_ACCEPTOR_H
#define LADDER_ACCEPTOR_H

#include <utils.h>
#include <functional>
#include <memory>

#include <Base.h>

namespace ladder {

class Channel;
class SocketAddr;
using NewConnectionCallback = std::function<void(int, SocketAddr&&)>;

class Acceptor {
 public:
  Acceptor(const ChannelPtr&, bool ipv6);
  void SetNewConnectionCallback(const NewConnectionCallback& callback);

 private:
  void HandleAcceptCallback();

  ChannelPtr channel_;
  NewConnectionCallback new_connection_callback_;
  bool ipv6_;
};

}  // namespace ladder

#endif
