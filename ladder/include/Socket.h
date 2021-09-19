#ifndef LADDER_SOCKET_ADDR_H
#define LADDER_SOCKET_ADDR_H

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <string>

#include <utils.h>

namespace ladder {

typedef union {
  struct sockaddr_in addr_;
  struct sockaddr_in6 addr6_;
} sockaddr_t;

class SocketAddr {
public:
  SocketAddr(bool ipv6=true);
  SocketAddr(sockaddr_t* addr, bool ipv6=true);
  SocketAddr(const std::string& ip,
             uint16_t port, bool ipv6=true);
  void Bind(int fd);
  std::string ip() const;
  uint16_t port() const;
  bool ipv6() const;
  const sockaddr_t* addr() const;

private:
  std::string ip_;
  uint16_t port_;
  sockaddr_t sa_;
  bool ipv6_;
};

namespace socket {

int socket(bool tcp = true, bool ipv6 = true);
int listen(int fd);
int accept(int fd, sockaddr_t* addr, socklen_t* addr_len);
int connect(int fd, const sockaddr_t* addr, socklen_t addr_len);
int shutdown_write(int fd);
int shutdown_read(int fd);
int getsockname(int fd, sockaddr_t* addr, socklen_t* addr_len);

} // namespace socket

} // namespace ladder

#endif
