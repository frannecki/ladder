#ifndef LADDER_SOCKET_ADDR_H
#define LADDER_SOCKET_ADDR_H

#include <stdint.h>
#ifdef __unix__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#ifdef _MSC_VER
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#endif
#include <fcntl.h>

#include <string>

#include <openssl/ssl.h>

#include <utils.h>

namespace ladder {

typedef union {
  struct sockaddr_in addr_;
  struct sockaddr_in6 addr6_;
} sockaddr_t;

class SocketAddr {
 public:
  SocketAddr(bool ipv6 = true);
  SocketAddr(sockaddr_t* addr, bool ipv6 = true);
  SocketAddr(const std::string& ip, uint16_t port, bool ipv6 = true);
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
int write(int fd, const void* buf, size_t len);
int read(int fd, void* buf, size_t len);
#ifdef __unix__
int sendfile(int out_fd, int in_fd, off_t* offset, size_t count);
#elif defined(_MSC_VER)
int sendfile(int out_fd, HANDLE in_fd, off_t* offset, size_t count);
#endif
int shutdown_write(int fd);
int shutdown_read(int fd);
int close(int fd);
int getsockname(int fd, sockaddr_t* addr, socklen_t* addr_len);
int getsockerropt(int fd);

}  // namespace socket

}  // namespace ladder

#endif
