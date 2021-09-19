#include <Connector.h>
#include <Channel.h>

#include <Logger.h>

namespace ladder {

Connector::Connector(const ChannelPtr& channel, int max_retry, bool ipv6) :
  channel_(channel), retry_(0), max_retry_(max_retry), ipv6_(ipv6)
{

}

void Connector::SetConnectionCallback(const ConnectionCallback& callback) {
  connection_callback_ = callback;
}

void Connector::SetConnectionFailureCallback(const ConnectionFailureCallback& callback) {
  connection_failure_callback_ = callback;
}

void Connector::Start(const SocketAddr& addr) {
  channel_->SetWriteCallback(std::bind(&Connector::HandleConnect, this));
  const sockaddr_t* sa = addr.addr();
  while(1) {
    int ret = socket::connect(channel_->fd(), sa,
                              ipv6_ ? sizeof(sa->addr6_) : sizeof(sa->addr_));
    if(ret < 0) {
      switch(errno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
          break;

#if EWOULDBLOCK != EAGAIN
        case(EWOULDBLOCK):
#endif
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
          if((++retry_) <= max_retry_) {
            Retry();
          }
          break;

        // case EACCES:
        // case EPERM:
        // case EAFNOSUPPORT:
        // case EALREADY:
        // case EBADF:
        // case EFAULT:
        // case ENOTSOCK:
        //   break;

        default:
          LOG_FATAL("Failure with ::connect. fd = " + std::to_string(channel_->fd()) + \
                    " errno = " + std::to_string(errno));
          perror("connect");
      }
    }
  }
}

void Connector::HandleConnect() {
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
  socket::getsockname(channel_->fd(), &addr, &addr_len);
  SocketAddr sock_addr(&addr);
  if(connection_callback_) {
    connection_callback_(std::move(sock_addr));
  }
}

void Retry() {
  // TODO: retry connect
}

} // namespace ladder
