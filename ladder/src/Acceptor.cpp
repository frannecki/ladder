#include <string.h>

#include <functional>

#ifdef _MSC_VER
#include <winsock2.h>
#include <mswsock.h>
#endif

#include <Acceptor.h>
#include <Channel.h>
#include <utils.h>

namespace ladder {

static thread_local LPFN_ACCEPTEX fn_acceptex = nullptr;

Acceptor::Acceptor(const ChannelPtr& channel, bool ipv6)
    : channel_(channel), ipv6_(ipv6) {
#ifdef _MSC_VER
  channel->set_read_callback(
      std::bind(&Acceptor::HandleAcceptCallback, this, std::placeholders::_1));
  accept_buffer_ = new char[kMaxIocpRecvSize];
  cur_accept_fd_ = 0;
  status_ = new SocketIocpStatus(kPollIn);
#else
  channel->set_read_callback(std::bind(&Acceptor::HandleAcceptCallback, this));
#endif
}

Acceptor::~Acceptor() {
  delete status_;
  delete accept_buffer_;
}

void Acceptor::Init() {
#ifdef _MSC_VER
  HandleAcceptCallback();
#endif
}

void Acceptor::set_new_connection_callback(const NewConnectionCallback& callback) {
  new_connection_callback_ = callback;
}

#ifdef _MSC_VER
void Acceptor::HandleAcceptCallback(int io_size) {
  if (cur_accept_fd_ != 0) {
    SOCKET enable = channel_->fd();
    int ret = setsockopt(cur_accept_fd_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                         (char*)&enable, sizeof(enable));
    if (ret == SOCKET_ERROR) {
      EXIT("setsockopt error: %d", WSAGetLastError());
    }

    sockaddr_t addr;
    socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
    socket::getpeername(cur_accept_fd_, &addr, &addr_len);
    SocketAddr sock_addr(&addr, ipv6_);
    if (new_connection_callback_) {
      new_connection_callback_(cur_accept_fd_, sock_addr, accept_buffer_, io_size);
    }
  }
  if (!fn_acceptex) {
    DWORD bytes = 0;
    GUID guid = WSAID_ACCEPTEX;
    int ret =
        WSAIoctl(channel_->fd(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                 &fn_acceptex, sizeof(fn_acceptex), &bytes, NULL, NULL);
    if (ret == SOCKET_ERROR) {
      EXIT("WSAIoctl");
    }
  }
  status_->Reset();
  cur_accept_fd_ = socket::accept(channel_->fd(), accept_buffer_, fn_acceptex, status_, ipv6_);
}
#else
void Acceptor::HandleAcceptCallback() {
  sockaddr_t addr;
  socklen_t addr_len = ipv6_ ? sizeof(addr.addr6_) : sizeof(addr.addr_);
  bzero(&addr, sizeof(addr));
  int fd = socket::accept(channel_->fd(), &addr, &addr_len);
  SocketAddr sock_addr(&addr, ipv6_);
  if (new_connection_callback_) {
    new_connection_callback_(fd, std::move(sock_addr));
  }
}
#endif

}  // namespace ladder
