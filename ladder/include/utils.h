#ifndef LADDER_UTILS_H
#define LADDER_UTILS_H

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

namespace ladder {

typedef union {
  struct sockaddr_in addr_;
  struct sockaddr_in6 addr6_;
} sockaddr_t;

#define EXIT exit_fatal

void exit_fatal(const char* msg);

class SocketAddr {
public:
  SocketAddr(const std::string& ip,
             uint16_t port, bool ipv6=false);
  int Bind(int fd);
  std::string ip() const;
  uint16_t port() const;

private:
  std::string ip_;
  uint16_t port_;
  sockaddr_t sa_;
  bool ipv6_;
};

} // namespace ladder

#endif
