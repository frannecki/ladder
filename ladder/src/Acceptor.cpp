#include <string.h>

#include <functional>

#include <Acceptor.h>
#include <Channel.h>
#include <utils.h>
#include <Socket.h>
#include <ThreadPool.h>

#include <Logger.h>

namespace ladder {

Acceptor::Acceptor(const ChannelPtr& channel, const ThreadPoolPtr& working_threads, bool ipv6) :
  channel_(channel), working_threads_(working_threads), ipv6_(ipv6)
{
  channel->SetReadCallback(std::bind(&Acceptor::HandleAccept, this));
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& callback) {
  new_connection_callback_ = callback;
}

void Acceptor::HandleAccept() {
  working_threads_->emplace(std::bind(&Acceptor::HandleAcceptCallback, this));
}

void Acceptor::HandleAcceptCallback() {
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
  bzero(&addr, sizeof(addr));
  int fd = socket::accept(channel_->fd(), &addr, &addr_len);

  SocketAddr sock_addr(&addr, ipv6_);
  if(new_connection_callback_) {
    new_connection_callback_(fd, sock_addr);
  }
}

} // namespace ladder
