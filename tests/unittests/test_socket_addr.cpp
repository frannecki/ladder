#include <string>

#include <Socket.h>

#include "ladtest/ladtest.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace ladder;

class SocketAddrTest : public Test {
 public:
  static void SetUpTestSuite() {
    fd = ladder::socket::socket(true, false);
    addr = new ladder::SocketAddr("127.0.0.1", 8086, false);
    addr6 = new ladder::SocketAddr("127.0.0.1", 8087, true);
  }

  static void TearDownTestSuite() {
    if (addr) {
      delete addr;
    }
    if (addr6) {
      delete addr6;
    }
    ladder::socket::close(fd);
  }

  static ladder::SocketAddr* addr;
  static ladder::SocketAddr* addr6;
  static int fd;
};

ladder::SocketAddr* SocketAddrTest::addr = nullptr;
ladder::SocketAddr* SocketAddrTest::addr6 = nullptr;
int SocketAddrTest::fd = -1;

TEST_F(SocketAddrTest, test_ipv4) {
  EXPECT_EQ(addr->ip(), "127.0.0.1");
  EXPECT_EQ(addr->port(), 8086);
  const ladder::sockaddr_t* sa = addr->addr();
  EXPECT_EQ(sa->addr_.sin_family, AF_INET);
  EXPECT_EQ(sa->addr_.sin_port, htons(8086));
  EXPECT_EQ(sa->addr_.sin_addr.s_addr, htonl(0x7f000001));
}

TEST_F(SocketAddrTest, test_ipv6) {
  EXPECT_EQ(addr6->ip(), "127.0.0.1");
  EXPECT_EQ(addr6->port(), 8087);
  const ladder::sockaddr_t* sa = addr6->addr();
  EXPECT_EQ(sa->addr6_.sin6_family, AF_INET6);
  EXPECT_EQ(sa->addr6_.sin6_port, htons(8087));
}
