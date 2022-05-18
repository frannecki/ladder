#ifndef LADDER_ACCEPTOR_H
#define LADDER_ACCEPTOR_H

#include <functional>
#include <memory>

#include "Base.h"
#include "Socket.h"

namespace ladder {

class Channel;
class SocketAddr;
#ifdef LADDER_OS_WINDOWS
using NewConnectionCallback = std::function<void(int, SocketAddr&&, char*, int)>;
#else
using NewConnectionCallback = std::function<void(int, SocketAddr&&)>;
#endif

class Acceptor {
 public:
  Acceptor(const ChannelPtr&, bool ipv6);
  Acceptor(const Acceptor&) = delete;
  Acceptor& operator=(const Acceptor&) = delete;
  ~Acceptor();
  void Init();
  void SetNewConnectionCallback(const NewConnectionCallback& callback);

 private:
#ifdef LADDER_OS_WINDOWS
  void HandleAcceptCallback(int io_size = 0);
  char* accept_buffer_;
  int cur_accept_fd_;
  SocketIocpStatus* status_;
#else
  void HandleAcceptCallback();
#endif

  ChannelPtr channel_;
  NewConnectionCallback new_connection_callback_;
  bool ipv6_;
};

}  // namespace ladder

#endif
