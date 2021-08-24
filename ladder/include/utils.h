#ifndef LADDER_UTILS_H
#define LADDER_UTILS_H

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <string>

namespace ladder {

const int kEnableOption = 1;
typedef union {
  struct sockaddr_in addr_;
  struct sockaddr_in6 addr6_;
} sockaddr_t;

#define EXIT exit_fatal

void exit_fatal(const char* msg);

class SocketAddr {
public:
  SocketAddr(bool ipv6=true);
  SocketAddr(sockaddr_t* addr, bool ipv6=true);
  SocketAddr(const std::string& ip,
             uint16_t port, bool ipv6=true);
  void Bind(int fd);
  std::string ip() const;
  uint16_t port() const;
  const sockaddr_t* addr();

private:
  std::string ip_;
  uint16_t port_;
  sockaddr_t sa_;
  bool ipv6_;
};

namespace socket {

int socket(bool tcp = true, bool ipv6 = true);
int accept(int fd, sockaddr_t* addr);
int shutdown_write(int fd);
int shutdown_read(int fd);

} // namespace socket

} // namespace ladder

#endif
