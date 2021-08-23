#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <TcpServer.h>
#include <Channel.h>
#include <EventLoop.h>
#include <EventLoopThreadPool.h>
#include <Acceptor.h>
#include <Connection.h>

using namespace std::placeholders;

namespace ladder {

TcpServer::TcpServer(const SocketAddr& addr, bool ipv6) : 
  loop_(std::make_shared<EventLoop>()), ipv6_(ipv6)
{

}

TcpServer::~TcpServer() {
  ::close(channel_->fd());
}

void TcpServer::Start() {
  int fd = socket::socket(true, ipv6_);
  channel_.reset(new Channel(loop_, fd));
  acceptor_.reset(new Acceptor(channel_));
  acceptor_->SetNewConnectionCallback(
    std::bind(&TcpServer::OnNewConnectionCallback, 
      this, std::placeholders::_1));
  loop_->StartLoop();
}

void TcpServer::SetReadCallback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void TcpServer::SetWriteCallback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void TcpServer::OnNewConnectionCallback(int fd) {
  EventLoopPtr loop = pool_->GetNextLoop();
  ConnectionPtr connection = std::make_shared<Connection>(loop, fd);
  connection->SetReadCallback(read_callback_);
  connection->SetWriteCallback(write_callback_);
  connections_.insert(std::pair<int, ConnectionPtr>(fd, connection));
}

EventLoopPtr TcpServer::loop() const {
  return loop_;
}

} // namespace ladder
