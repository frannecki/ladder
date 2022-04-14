#include <compat.h>
#ifdef LADDER_OS_UNIX
#include <unistd.h>
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
# define SOCK_NONBLOCK O_NONBLOCK
#endif
#endif
#ifdef LADDER_OS_LINUX
#include <sys/sendfile.h>
#endif
#ifdef LADDER_OS_WINDOWS
#pragma comment(lib, "mswsock.lib")
#endif
#include <string.h>

#include <mutex>

#include <Socket.h>
#include <utils.h>

namespace ladder {

SocketAddr::SocketAddr(bool ipv6) : ipv6_(ipv6) {}

SocketAddr::SocketAddr(const sockaddr_t* addr, bool ipv6) : ipv6_(ipv6) {
  char buf[50];
  memcpy(&sa_, addr, sizeof(sa_));
  if (ipv6) {
    if (inet_ntop(AF_INET6, &(addr->addr6_.sin6_addr), buf, sizeof(buf)) ==
        NULL) {
      EXIT("inet_pton");
    }
    ip_ = std::string(buf);
    port_ = ntohs(addr->addr6_.sin6_port);
  } else {
    if (inet_ntop(AF_INET, &(addr->addr_.sin_addr), buf, sizeof(buf)) == NULL) {
      EXIT("inet_pton");
    }
    ip_ = std::string(buf);
    port_ = ntohs(addr->addr_.sin_port);
  }
}

SocketAddr::SocketAddr(const std::string& ip, uint16_t port, bool ipv6)
    : ip_(ip), port_(port), ipv6_(ipv6) {
  if (ipv6) {
    sa_.addr6_.sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, ip.c_str(), &sa_.addr6_.sin6_addr) < 0) {
      EXIT("inet_pton");
    }
    sa_.addr6_.sin6_port = htons(port);
  } else {
    sa_.addr_.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &sa_.addr_.sin_addr) < 0) {
      EXIT("inet_pton");
    }
    sa_.addr_.sin_port = htons(port);
  }
}

void SocketAddr::Bind(int fd) const {
  socklen_t sa_len = ipv6_ ? sizeof(sa_.addr6_) : sizeof(sa_.addr_);
  int ret = ::bind(fd, (struct sockaddr*)&sa_, sa_len);
  if (ret < 0) {
    EXIT("bind");
  }
}

std::string SocketAddr::ip() const { return ip_; }

uint16_t SocketAddr::port() const { return port_; }

bool SocketAddr::ipv6() const { return ipv6_; }

const sockaddr_t* SocketAddr::addr() const { return &sa_; }

namespace socket {

int socket(bool tcp, bool ipv6) {
#ifdef LADDER_OS_UNIX
  int fd = ::socket(ipv6 ? AF_INET6 : AF_INET,
                    (tcp ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
  if (fd < 0) {
        EXIT("socket");
  }
  int enable = kEnableOption;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
    EXIT("setsockopt");
  }
#endif
#ifdef LADDER_OS_WINDOWS
    int fd = WSASocket(ipv6 ? AF_INET6 : AF_INET,
                       tcp ? SOCK_STREAM : SOCK_DGRAM,
                       IPPROTO_IP,
                       NULL,
                       0,
                       WSA_FLAG_OVERLAPPED);
  if (fd == INVALID_SOCKET) {
    EXIT("socket");
  }
  int enable = kEnableOption;
  int zero = 0;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable)) ==
      SOCKET_ERROR) {
    EXIT("setsockopt SO_SNDBUG error: %d", WSAGetLastError());
  }
  if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero)) == SOCKET_ERROR) {
    EXIT("setsockopt SO_SNDBUG error: %d", WSAGetLastError());
  }
#endif
  return fd;
}

int listen(int fd) {
#ifdef LADDER_OS_WINDOWS
  int ret = ::listen(fd, SOMAXCONN);
  if (ret < 0) {
    EXIT("listen error: %d", WSAGetLastError());
  }
#else
  int ret = ::listen(fd, 0);
  if (ret < 0) {
    EXIT("listen");
  }
#endif
  return ret;
}

#ifdef LADDER_OS_WINDOWS
int accept(int fd, char* buffer, LPFN_ACCEPTEX fn_acceptex,
           SocketIocpStatus* status, bool ipv6) {
  int ret;
  int acceptfd = socket(true, ipv6);
  if (acceptfd == -1) return -1;
  
  status->Reset();

  DWORD bytes_recved = 0;

  socklen_t sa_len = ipv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
  ret = fn_acceptex(fd, acceptfd, (LPVOID)buffer,
                    kMaxIocpRecvSize - 2 * (sa_len + 16),
                    sa_len + 16, sa_len + 16,
                    &bytes_recved, (LPOVERLAPPED)status);
  if (ret == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
    EXIT("AcceptEx error: %d", WSAGetLastError());
  }

  return acceptfd;
}

int connect(int fd, const sockaddr_t* addr, socklen_t addr_len,
            LPFN_CONNECTEX fn_connectex,
            SocketIocpStatus* status, bool ipv6) {
  DWORD bytes_sent = 0;
  int ret = fn_connectex(fd, (const struct sockaddr*)addr, addr_len,
                         NULL, 0, &bytes_sent, (LPOVERLAPPED)status);
  if (ret == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
    EXIT("AcceptEx error: %d", WSAGetLastError());
  }
  return ret;
}

int write(int fd, LPWSABUF buf, SocketIocpStatus* status) {
  status->Reset();
  status->UpdateRefCount(true);
  DWORD bytes_sent = 0;
  int ret = WSASend(fd, buf, 1, &bytes_sent, 0, (LPWSAOVERLAPPED)status, NULL);
  return ret;
}

int read(int fd, LPWSABUF buf, SocketIocpStatus* status) {
  status->Reset();
  status->UpdateRefCount(true);
  DWORD bytes_recved = 0;
  DWORD flags = 0;
  buf->len = kMaxIocpRecvSize;

  int ret =
      WSARecv(fd, buf, 1, &bytes_recved, &flags, (LPWSAOVERLAPPED)status, NULL);

  return ret;
}

int sendfile(int out_fd, HANDLE in_fd, off_t* offset, size_t count) {
  // TODO: asynchronous send with iocp
  // bool success = TransmitFile(out_fd, in_fd, 0, 0, 0, NULL, 0);
  return -1;
}
#else


bool fcntl_set_option(int sock, int nonblock, int cloexec) {
    int opts, ret;

    if (nonblock >= 0) {
        do {
            opts = fcntl(sock, F_GETFL);
        } while (opts < 0 && errno == EINTR);

        if (opts < 0) {
          EXIT("fcntl(n, GETFL) failed");
        }

        if (nonblock) {
            opts = opts | O_NONBLOCK;
        } else {
            opts = opts & ~O_NONBLOCK;
        }

        do {
            ret = fcntl(sock, F_SETFL, opts);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            EXIT("fcntl(n, SETFL, opts) failed");
            return false;
        }
    }

#ifdef FD_CLOEXEC
    if (cloexec >= 0) {
        do {
            opts = fcntl(sock, F_GETFD);
        } while (opts < 0 && errno == EINTR);

        if (opts < 0) {
            EXIT("fcntl(n, GETFL) failed");
        }

        if (cloexec) {
            opts = opts | FD_CLOEXEC;
        } else {
            opts = opts & ~FD_CLOEXEC;
        }

        do {
            ret = fcntl(sock, F_SETFD, opts);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            EXIT("fcntl(n, SETFD, opts) failed");
            return false;
        }
    }
#endif

    return true;
}

int accept(int fd, sockaddr_t* addr, socklen_t* addr_len) {
#ifndef LADDER_OS_MAC
  int accepted = ::accept4(fd, (struct sockaddr*)addr, addr_len, SOCK_NONBLOCK);
#else
  int accepted = ::accept(fd, (struct sockaddr*)addr, addr_len);
  if (accepted >= 0) {
      fcntl_set_option(fd, 1, 1);
  }
#endif
  if (accepted < 0) {
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

#ifdef LADDER_OS_UNIX
int sendfile(int out_fd, int in_fd, off_t* offset, size_t count) {
#ifdef LADDER_OS_LINUX
  int ret = ::sendfile(out_fd, in_fd, offset, count);
#elif defined(LADDER_HAVE_KQUEUE)
  off_t bytes_sent;
#ifdef LADDER_OS_MAC
  int success = ::sendfile(in_fd, out_fd, *offset, (off_t*)count, NULL, 0);
#elif LADDER_OS_FREEBSD
  int success = ::sendfile(in_fd, out_fd, *offset, count, NULL, &bytes_sent, 0);
#endif
  int ret = (success == 0) ? static_cast<int>(bytes_sent) : 0;
#endif
  return ret;
}
#endif

#endif

int shutdown_write(int fd) {
#ifdef LADDER_OS_UNIX
  int ret = ::shutdown(fd, SHUT_WR);
#endif
#ifdef LADDER_OS_WINDOWS
  int ret = ::shutdown(fd, SD_SEND);
#endif
  if (ret < 0) {
    switch (errno) {
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
#ifdef LADDER_OS_UNIX
  int ret = ::shutdown(fd, SHUT_RD);
#endif
#ifdef LADDER_OS_WINDOWS
  int ret = ::shutdown(fd, SD_RECEIVE);
#endif
  
  if (ret < 0) {
    EXIT("shutdown SHUT_RD");
  }
  return ret;
}

int close(int fd) {
#ifdef LADDER_OS_UNIX
  int ret = ::close(fd);
  if (ret < 0) {
#endif
#ifdef LADDER_OS_WINDOWS
  int ret = ::closesocket(fd);
  if (ret == SOCKET_ERROR) {
#endif
    EXIT("close");
  }
  return ret;
}

int getsockname(int fd, sockaddr_t* addr, socklen_t* addr_len) {
  int ret = ::getsockname(fd, (struct sockaddr*)addr, addr_len);
  if (ret < 0) {
#ifdef LADDER_OS_WINDOWS
    EXIT("getsockname error: %d", WSAGetLastError());
#else
    EXIT("getsockname");
#endif
  }
  return ret;
}

int getpeername(int fd, sockaddr_t* addr, socklen_t* addr_len) {
  int ret = ::getpeername(fd, (struct sockaddr*)addr, addr_len);
  if (ret < 0) {
#ifdef LADDER_OS_WINDOWS
    EXIT("getpeername error: %d", WSAGetLastError());
#else
    EXIT("getpeername");
#endif
  }
  return ret;
}

int getsockerropt(int fd) {
  int err;
  socklen_t len = sizeof(int);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &len);
  return err;
}

}  // namespace socket

}  // namespace ladder
