#include <Connector.h>
#include <Channel.h>
#include <Timer.h>

#include <Logger.h>

namespace ladder {

Connector::Connector(const ChannelPtr& channel,
                     int max_retry,
                     const SocketAddr& addr,
                     uint16_t retry_initial_timeout) :
  channel_(channel), retry_(0), max_retry_(max_retry),
  retry_timeout_(std::max(retry_initial_timeout, kMinRetryInitialTimeout)),
  timer_(new Timer(channel->loop())), ipv6_(addr.ipv6()), addr_(addr)
{
  timer_->SetTimerEventCallback(std::bind(&Connector::Start, this));
}

void Connector::SetConnectionCallback(const ConnectionCallback& callback) {
  connection_callback_ = callback;
}

void Connector::SetConnectionFailureCallback(const ConnectionFailureCallback& callback) {
  connection_failure_callback_ = callback;
}

void Connector::Start() {
  channel_->SetWriteCallback(std::bind(&Connector::HandleConnect, this));
  channel_->SetErrorCallback(std::bind(&Connector::Retry, this));
  const sockaddr_t* sa = addr_.addr();
  int ret = socket::connect(channel_->fd(), sa,
                            ipv6_ ? sizeof(sa->addr6_) : sizeof(sa->addr_));
  if(ret < 0) {
    switch(errno) {
      case 0:
      case EINPROGRESS:
      case EINTR:
      case EISCONN:
        if(socket::getsockerropt(channel_->fd()) == 0) {
          break;
        }

#if EWOULDBLOCK != EAGAIN
      case(EWOULDBLOCK):
#endif
      case EAGAIN:
      case EADDRINUSE:
      case EADDRNOTAVAIL:
      case ECONNREFUSED:
      case ENETUNREACH:
        Retry();
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
        Retry();
    }
  }
}

void Connector::HandleConnect() {
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
  socket::getsockname(channel_->fd(), &addr, &addr_len);
  SocketAddr sock_addr(&addr, ipv6_);
  if(connection_callback_) {
    connection_callback_(std::move(sock_addr));
  }
}

void Connector::Retry() {
  if((retry_ += 1) <= max_retry_) {
    LOG_DEBUG("Connect failed. Trying again after " + std::to_string(retry_timeout_) + " ms.");
    // TODO: retry connect
    timer_->SetInterval(retry_timeout_ * 1000, false);
    retry_timeout_ <<= 1;
  }
  else {
    if(connection_failure_callback_) {
      connection_failure_callback_();
    }
  }
}

} // namespace ladder
