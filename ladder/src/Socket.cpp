#include <unistd.h>

#include <mutex>

#include <Socket.h>
#include <utils.h>

namespace ladder {

SocketAddr::SocketAddr(bool ipv6) : 
  ipv6_(ipv6)
{
  ;
}

SocketAddr::SocketAddr(sockaddr_t* addr, bool ipv6) :
  ipv6_(ipv6)
{
  char buf[50];
  if(ipv6) {
    if(inet_ntop(AF_INET6, &(addr->addr6_.sin6_addr), buf, sizeof(buf)) < 0) {
      EXIT("inet_pton");
    }
    ip_ = std::string(buf);
    port_ = ntohs(addr->addr6_.sin6_port);
  }
  else {
    if(inet_ntop(AF_INET, &(addr->addr_.sin_addr), buf, sizeof(buf)) < 0) {
      EXIT("inet_pton");
    }
    ip_ = std::string(buf);
    port_ = ntohs(addr->addr_.sin_port);
  }
}

SocketAddr::SocketAddr(const std::string& ip, uint16_t port, bool ipv6) : 
  ip_(ip), port_(port), ipv6_(ipv6)
{
  if(ipv6) {
    sa_.addr6_.sin6_family = AF_INET6;
    if(inet_pton(AF_INET6, ip.c_str(), &sa_.addr6_.sin6_addr) < 0) {
      EXIT("inet_pton");
    }
    sa_.addr6_.sin6_port = htons(port);
  }
  else {
    sa_.addr_.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip.c_str(), &sa_.addr_.sin_addr) < 0) {
      EXIT("inet_pton");
    }
    sa_.addr_.sin_port = htons(port);
  }
}

void SocketAddr::Bind(int fd) {
  socklen_t sa_len;
  if(ipv6_) {
    sa_len = sizeof(sa_.addr6_);
  }
  else {
    sa_len = sizeof(sa_.addr_);
  }
  int ret = ::bind(fd, (struct sockaddr*)&sa_, sa_len);
  if(ret < 0) {
    EXIT("bind");
  }
}

std::string SocketAddr::ip() const {
  return ip_;
}
  
uint16_t SocketAddr::port() const {
  return port_;
}

bool SocketAddr::ipv6() const {
  return ipv6_;
}

const sockaddr_t* SocketAddr::addr() const {
  return &sa_;
}

namespace socket {

int socket(bool tcp, bool ipv6) {
  int fd = ::socket(ipv6 ? AF_INET6 : AF_INET,
                    (tcp ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
  if(fd < 0) {
    EXIT("socket");
  }
  int enable = kEnableOption;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
    EXIT("setsockopt");
  }
  return fd;
}

int listen(int fd) {
  int ret = ::listen(fd, 0);
  if(ret < 0) {
    EXIT("listen");
  }
  return ret;
}

int accept(int fd, sockaddr_t* addr, socklen_t* addr_len) {
  int accepted = ::accept4(fd, (struct sockaddr*)addr, addr_len, SOCK_NONBLOCK);
  if(accepted < 0) {
    EXIT("accept");
  }
  return accepted;
}

int connect(int fd, const sockaddr_t* addr, socklen_t addr_len) {
  int ret = ::connect(fd, (const struct sockaddr*)addr, addr_len);
  return ret;
}

int write(int fd, const void* buf, size_t len) {
  return ::write(fd, buf, len);
}

int read(int fd, void* buf, size_t len) {
  return ::read(fd, buf, len);
}

int shutdown_write(int fd) {
  int ret = ::shutdown(fd, SHUT_WR);
  if(ret < 0) {
    switch(errno) {
      case ENOTCONN:
        break;
      default:
        // EXIT("shutdown SHUT_WR");
        break;
    }
  }
  return ret;
}

int shutdown_read(int fd) {
  int ret = ::shutdown(fd, SHUT_RD);
  if(ret < 0) {
    EXIT("shutdown SHUT_RD");
  }
  return ret;
}

int close(int fd) {
  int ret = ::close(fd);
  if(ret < 0) {
    EXIT("close");
  }
  return ret;
}

int getsockname(int fd, sockaddr_t* addr, socklen_t* addr_len) {
  int ret = ::getsockname(fd, (struct sockaddr*)addr, addr_len);
  if(ret < 0) {
    EXIT("getsockname");
  }
  return ret;
}

int getsockerropt(int fd) {
  int err;
  socklen_t len = sizeof(int);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
  return err;
}


} // socket

} // namespace ladder
