#include <Channel.h>
#include <Logging.h>
#include <utils.h>

#include <Connector.h>
#include <Timer.h>

namespace ladder {

Connector::Connector(const ChannelPtr& channel, int max_retry,
                     const SocketAddr& addr, uint16_t retry_initial_timeout)
    : channel_(channel),
      retry_(0),
      max_retry_(max_retry),
      retry_timeout_(
          (std::max)(retry_initial_timeout, kMinRetryInitialTimeout)),
#ifdef LADDER_OS_WINDOWS
      status_(new SocketIocpStatus(kPollOut)),
      timer_(new Timer),
#else
      timer_(new Timer(channel->loop())),
#endif
      ipv6_(addr.ipv6()),
      addr_(addr) {
  timer_->SetTimmerEventCallback(std::bind(&Connector::Start, this));
}

#ifdef LADDER_OS_WINDOWS
static thread_local LPFN_CONNECTEX fn_connectex = nullptr;

Connector::~Connector() { 
  delete status_;
}

#else

Connector::~Connector() {}

#endif

void Connector::SetConnectionCallback(const ConnectionCallback& callback) {
  connection_callback_ = callback;
}

void Connector::SetConnectionFailureCallback(
    const ConnectionFailureCallback& callback) {
  connection_failure_callback_ = callback;
}

void Connector::Start() {
  channel_->SetWriteCallback(std::bind(&Connector::HandleConnect, this));
  channel_->SetErrorCallback(std::bind(&Connector::Retry, this));
  channel_->SetCloseCallback(std::bind(&Connector::Retry, this));
  channel_->SetReadCallback(nullptr);
  channel_->EnableWrite(true);
  const sockaddr_t* sa = addr_.addr();
#ifdef LADDER_OS_WINDOWS
  if (!fn_connectex) {
    DWORD bytes = 0;
    GUID guid = WSAID_CONNECTEX;
    int ret = WSAIoctl(channel_->fd(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                       &guid, sizeof(guid), &fn_connectex, sizeof(fn_connectex),
                       &bytes, NULL, NULL);
    if (ret == SOCKET_ERROR) {
      EXIT("WSAIoctl error: %d", GetLastError());
    }
  }
  socket::connect(channel_->fd(), sa,
                  ipv6_ ? sizeof(sa->addr6_) : sizeof(sa->addr_),
                  fn_connectex, status_, ipv6_);
#else
  int ret = socket::connect(channel_->fd(), sa,
                            ipv6_ ? sizeof(sa->addr6_) : sizeof(sa->addr_));
  if (ret < 0) {
    switch (errno) {
      case 0:
      case EINPROGRESS:
      case EINTR:
      case EISCONN:
        break;

#if EWOULDBLOCK != EAGAIN
      case (EWOULDBLOCK):
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
        LOGF_FATAL("Failure with ::connect. fd = %d. errno = %d",
                   channel_->fd(), errno);
        perror("connect");
        Retry();
    }
  }
#endif
}

void Connector::HandleConnect() {
  if (socket::getsockerropt(channel_->fd())) {
    Retry();
    return;
  }
#ifdef LADDER_OS_WINDOWS
  int ret = setsockopt(channel_->fd(), SOL_SOCKET,
                       SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
  if (ret == SOCKET_ERROR) {
    EXIT("setsockopt error: %d", WSAGetLastError());
  }
#endif
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
  socket::getsockname(channel_->fd(), &addr, &addr_len);
  SocketAddr sock_addr(&addr, ipv6_);
  if (connection_callback_) {
    connection_callback_(std::move(sock_addr));
  }
}

void Connector::Retry() {
  if ((retry_ += 1) <= max_retry_) {
    LOG_DEBUG("Connect failed. Trying again after " +
              std::to_string(retry_timeout_) + " ms.");
    // TODO: retry connect
    timer_->SetInterval(retry_timeout_ * 1000, false);
    retry_timeout_ <<= 1;
  } else {
    if (connection_failure_callback_) {
      connection_failure_callback_();
    }
  }
}

}  // namespace ladder
