#include <utils.h>

namespace ladder {

void exit_fatal(const char* msg) {
  perror(msg);
  exit(-1);
}

SocketAddr::SocketAddr(const std::string& ip, uint16_t port, bool ipv6) : 
  ip_(ip), port_(port), ipv6_(ipv6)
{
  if(ipv6_) {
    sa_.addr6_.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip.c_str(), &sa_.addr6_.sin6_addr);
    sa_.addr6_.sin6_port = port;
  }
  else {
    sa_.addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &sa_.addr_.sin_addr);
    sa_.addr_.sin_port = port;
  }
}

int SocketAddr::Bind(int fd) {
  socklen_t sa_len;
  if(ipv6_) {
    sa_len = sizeof(sa_.addr6_);
  }
  else {
    sa_len = sizeof(sa_.addr_);
  }
  return ::bind(fd, (struct sockaddr*)&sa_, sa_len);
}

std::string SocketAddr::ip() const {
  return ip_;
}
  
uint16_t SocketAddr::port() const {
  return port_;
}

} // namespace ladder
