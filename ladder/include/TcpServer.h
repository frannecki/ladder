#ifndef LADDER_TCP_SERVER_H
#define LADDER_TCP_SERVER_H

#include <utils.h>

namespace ladder {

class Channel;

class TcpServer {
public:
  TcpServer(const SocketAddr& addr);
  const Channel* channel() const;

private:
  Channel* channel_;
};

};

#endif
