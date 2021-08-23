#include <utils.h>

namespace ladder {

void exit_fatal(const char* msg) {
  perror(msg);
  exit(-1);
}

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
    if(inet_ntop(AF_INET6, &(addr->addr6_), buf, sizeof(buf)) < 0) {
      exit_fatal("inet_pton");
    }
    ip_ = std::string(buf);
    port_ = ntohs(addr->addr6_.sin6_port);
  }
  else {
    if(inet_ntop(AF_INET, &(addr->addr_), buf, sizeof(buf)) < 0) {
      exit_fatal("inet_pton");
    }
    ip_ = std::string(buf);
    port_ = ntohs(addr->addr_.sin_port);
  }
}

SocketAddr::SocketAddr(const std::string& ip, uint16_t port, bool ipv6) : 
  ip_(ip), port_(port), ipv6_(ipv6)
{
  if(ipv6_) {
    sa_.addr6_.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip.c_str(), &sa_.addr6_.sin6_addr);
    sa_.addr6_.sin6_port = htons(port);
  }
  else {
    sa_.addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &sa_.addr_.sin_addr);
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
    exit_fatal("bind");
  }
}

std::string SocketAddr::ip() const {
  return ip_;
}
  
uint16_t SocketAddr::port() const {
  return port_;
}

const sockaddr_t* SocketAddr::addr() {
  return &sa_;
}

namespace socket {

int socket(bool tcp, bool ipv6) {
  int fd = ::socket(ipv6 ? AF_INET6 : AF_INET,
                    tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
  if(fd < 0) {
    exit_fatal("socket");
  }
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & O_NONBLOCK);
  int enable = kEnableOption;
  if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))) {
    exit_fatal("setsockopt");
  }
  return fd;
}

int accept(int fd, sockaddr_t* addr) {
  socklen_t len = sizeof(sockaddr_t);
  int fd1 = ::accept(fd, (struct sockaddr*)addr, &len) < 0;
  if(fd1 < 0) {
    exit_fatal("accept");
  }
  return fd1;
}

} // socket

} // namespace ladder
