#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <TcpServer.h>
#include <Channel.h>
#include <EventLoop.h>
#include <EventLoopThreadPool.h>
#include <Acceptor.h>
#include <Connection.h>

#include <Logger.h>

using namespace std::placeholders;

namespace ladder {

TcpServer::TcpServer(const SocketAddr& addr) : 
  loop_(std::make_shared<EventLoop>()),
  addr_(addr)
{

}

TcpServer::~TcpServer() {
  ::close(channel_->fd());
}

void TcpServer::Start() {
  int fd = socket::socket(true, addr_.ipv6());
  addr_.Bind(fd);
  socket::listen(fd);
  channel_.reset(new Channel(loop_, fd));
  channel_->AddToLoop();
  acceptor_.reset(new Acceptor(channel_, addr_.ipv6()));
  acceptor_->SetNewConnectionCallback(
    std::bind(&TcpServer::OnNewConnectionCallback, 
              this, std::placeholders::_1, std::placeholders::_2));
  LOG_INFO("Listening on " + addr_.ip() + ":" + std::to_string(addr_.port()));
  thread_pool_.reset(new EventLoopThreadPool);
  loop_->StartLoop();
}

void TcpServer::SetReadCallback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void TcpServer::SetWriteCallback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void TcpServer::SetConnectionCallback(const ConnectionEvtCallback& callback) {
  connection_callback_ = callback;
}

void TcpServer::OnNewConnectionCallback(int fd, const SocketAddr& addr) {
  // EventLoopPtr loop = thread_pool_->GetNextLoop();
  EventLoopPtr loop = std::make_shared<EventLoop>();
  ConnectionPtr connection = std::make_shared<Connection>(loop, fd);
  connection->SetReadCallback(read_callback_);
  connection->SetWriteCallback(write_callback_);
  connection->SetCloseCallback(std::bind(&TcpServer::OnCloseConnectionCallback,
                               this, fd));
  connection->Init();
  connections_.insert(std::pair<int, ConnectionPtr>(fd, connection));
  // action on connection
  connection_callback_(connection);
  LOG_INFO("New connection from client " + addr.ip() + ":" \
           + std::to_string(addr.port()));
}

void TcpServer::OnCloseConnectionCallback(int fd) {
  auto iter = connections_.find(fd);
  if(iter != connections_.end()) {
    connections_.erase(iter);
  }
}

EventLoopPtr TcpServer::loop() const {
  return loop_;
}

} // namespace ladder
