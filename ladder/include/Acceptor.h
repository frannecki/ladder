#ifndef LADDER_ACCEPTOR_H
#define LADDER_ACCEPTOR_H

#include <memory>
#include <functional>

namespace ladder {

using NewConnectionCallback = std::function<void(int)>;

class Channel;
using ChannelPtr = std::shared_ptr<Channel>;

class Acceptor {
public:
  Acceptor(const ChannelPtr&);
  void SetNewConnectionCallback(const NewConnectionCallback& callback);

private:
  void HandleAccept();

  ChannelPtr channel_;
  NewConnectionCallback new_connection_callback_;
};

} // namespace ladder

#endif
