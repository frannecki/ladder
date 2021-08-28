#include <functional>

#include <Acceptor.h>
#include <Channel.h>
#include <utils.h>
#include <Socket.h>

namespace ladder {

Acceptor::Acceptor(const ChannelPtr& channel, bool ipv6) :
  channel_(channel), ipv6_(ipv6)
{
  channel->SetReadCallback(std::bind(&Acceptor::HandleAccept, this));
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& callback) {
  new_connection_callback_ = callback;
}

void Acceptor::HandleAccept() {
  sockaddr_t addr;
  int fd = socket::accept(channel_->fd(), &addr);
  SocketAddr sock_addr(&addr, ipv6_);
  new_connection_callback_(fd, sock_addr);
}

} // namespace ladder
