#ifndef LADDER_ACCEPTOR_H
#define LADDER_ACCEPTOR_H

#include <memory>
#include <functional>
#include <utils.h>

#include <Aliases.h>

namespace ladder {

class Channel;
class SocketAddr;
using NewConnectionCallback = std::function<void(int, const SocketAddr&)>;

class Acceptor {
public:
  Acceptor(const ChannelPtr&, const ThreadPoolPtr&, bool ipv6);
  void SetNewConnectionCallback(const NewConnectionCallback& callback);

private:
  void HandleAccept();
  void HandleAcceptCallback();

  ChannelPtr channel_;
  ThreadPoolPtr working_threads_;
  NewConnectionCallback new_connection_callback_;
  bool ipv6_;
};

} // namespace ladder

#endif
