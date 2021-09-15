#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <TcpServer.h>
#include <Channel.h>
#include <EventLoop.h>
#include <EventLoopThreadPool.h>
#include <Acceptor.h>
#include <Connection.h>

#include <Aliases.h>
#include <Logger.h>

using namespace std::placeholders;

namespace ladder {

using TcpConnectionCloseCallback = std::unique_ptr<std::function<void(TcpServer*, int)>>;

TcpServer::TcpServer(const SocketAddr& addr, size_t thread_num) : 
  addr_(addr),
  thread_num_(thread_num)
{

}

TcpServer::~TcpServer() {
  ::close(channel_->fd());
}

void TcpServer::Start() {
  loop_ = std::make_shared<EventLoop>();
  int fd = socket::socket(true, addr_.ipv6());
  addr_.Bind(fd);
  socket::listen(fd);
  channel_.reset(new Channel(loop_, fd));
  channel_->AddToLoop();
  acceptor_.reset(new Acceptor(channel_, addr_.ipv6()));
  acceptor_->SetNewConnectionCallback(
    std::bind(&TcpServer::OnNewConnectionCallback, 
              this,
              std::placeholders::_1,
              std::placeholders::_2));
  LOG_INFO("Listening on fd = " + std::to_string(fd) + " " + \
           addr_.ip() + ":" + std::to_string(addr_.port()));
  thread_pool_.reset(new EventLoopThreadPool(thread_num_));
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
  EventLoopPtr loop = thread_pool_->GetNextLoop();
  ConnectionPtr connection = std::make_shared<Connection>(loop, fd);
  connection->SetReadCallback(read_callback_);
  connection->SetWriteCallback(write_callback_);
  connection->SetCloseCallback(ConnectionCloseCallbackPtr(
    new std::function<void()>(std::bind(&TcpServer::OnCloseConnectionCallback,
                                        this, fd))));
  
  // std::function constructed from std::bind
  // would get destroyed once out of scope
  
  // connection->SetCloseCallback(std::bind(
  //   &TcpServer::OnCloseConnectionCallback,
  //   this,
  //   fd));
  connection->Init();
  {
    std::lock_guard<std::mutex> lock(mutex_connections_);
    connections_.insert(std::pair<int, ConnectionPtr>(fd, connection));
    LOG_INFO("New connection from client " + addr.ip() + ":" \
             + std::to_string(addr.port()) + " fd = " + std::to_string(fd) \
             + " Current client number: " + std::to_string(connections_.size()));
  }
  // action on connection
  if(connection_callback_) {
    connection_callback_(connection);
  }
}

void TcpServer::OnCloseConnectionCallback(int fd) {
  LOG_INFO("Client end has closed write: " + std::to_string(fd));
  {
    std::lock_guard<std::mutex> lock(mutex_connections_);
    auto iter = connections_.find(fd);
    if(iter != connections_.end()) {
      connections_.erase(iter);
    }
  }
}

EventLoopPtr TcpServer::loop() const {
  return loop_;
}

} // namespace ladder
