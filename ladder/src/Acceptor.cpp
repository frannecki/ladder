#include <string.h>

#include <functional>

#include <Acceptor.h>
#include <Channel.h>
#include <Socket.h>
#include <utils.h>

namespace ladder {

Acceptor::Acceptor(const ChannelPtr& channel, bool ipv6)
    : channel_(channel), ipv6_(ipv6) {
  channel->set_read_callback(std::bind(&Acceptor::HandleAcceptCallback, this));
}

void Acceptor::set_new_connection_callback(const NewConnectionCallback& callback) {
  new_connection_callback_ = callback;
}

void Acceptor::HandleAcceptCallback() {
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
#ifdef __unix__
  bzero(&addr, sizeof(addr));
#endif
#ifdef _MSC_VER
  ZeroMemory(&addr, sizeof(addr));
#endif
  int fd = socket::accept(channel_->fd(), &addr, &addr_len);
  SocketAddr sock_addr(&addr, ipv6_);
  if (new_connection_callback_) {
    new_connection_callback_(fd, std::move(sock_addr));
  }
}

}  // namespace ladder
