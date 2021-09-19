#include <string.h>

#include <functional>

#include <Acceptor.h>
#include <Channel.h>
#include <utils.h>
#include <Socket.h>
#include <ThreadPool.h>

namespace ladder {

Acceptor::Acceptor(const ChannelPtr& channel, bool ipv6) :
  channel_(channel), ipv6_(ipv6)
{
  channel->SetReadCallback(std::bind(&Acceptor::HandleAcceptCallback, this));
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& callback) {
  new_connection_callback_ = callback;
}

void Acceptor::HandleAcceptCallback() {
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
  bzero(&addr, sizeof(addr));
  int fd = socket::accept(channel_->fd(), &addr, &addr_len);
  SocketAddr sock_addr(&addr, ipv6_);
  if(new_connection_callback_) {
    new_connection_callback_(fd, std::move(sock_addr));
  }
}

} // namespace ladder
