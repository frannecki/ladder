#ifndef LADDER_TCP_CLIENT_H
#define LADDER_TCP_CLIENT_H

#include <utils.h>

namespace ladder {

class Connection;
class SocketAddr;

class TcpClient {
public:
  TcpClient(const SocketAddr& addr);

private:
  Connection* conn_;
};

} // namespace ladder

#endif
