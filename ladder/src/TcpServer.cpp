#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <TcpServer.h>
#include <Channel.h>
#include <EventLoop.h>
#include <ThreadPool.h>
#include <EventLoopThreadPool.h>
#include <Acceptor.h>
#include <Connection.h>

#include <Aliases.h>
#include <Logger.h>

using namespace std::placeholders;

namespace ladder {

void signal_handler(int signum) {
  if(signum == SIGPIPE) {
    LOG_FATAL("SIGPIPE ignored");
  }
}

using TcpConnectionCloseCallback = std::unique_ptr<std::function<void(TcpServer*, int)>>;

TcpServer::TcpServer(const SocketAddr& addr,
                     size_t loop_thread_num,
                     size_t working_thread_num) : 
  addr_(addr),
  loop_thread_num_(loop_thread_num),
  working_thread_num_(working_thread_num)
{
  signal(SIGPIPE, signal_handler);
}

TcpServer::~TcpServer() {
  socket::close(channel_->fd());
  channel_->RemoveFromLoop();
}

void TcpServer::Start() {
  loop_ = std::make_shared<EventLoop>();
  working_threads_ = std::make_shared<ThreadPool>(working_thread_num_);
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
  LOGF_INFO("Listening on fd = %d %s:%u", fd,
            addr_.ip().c_str(), addr_.port());
  loop_threads_.reset(new EventLoopThreadPool(loop_thread_num_));
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

void TcpServer::OnNewConnectionCallback(int fd, SocketAddr&& addr) {
  working_threads_->emplace(std::bind(&TcpServer::OnNewConnection, this, fd, addr));
}

void TcpServer::OnNewConnection(int fd, const SocketAddr& addr) {
  EventLoopPtr loop = loop_threads_->GetNextLoop();
  ConnectionPtr connection = std::make_shared<Connection>(loop, fd);
  connection->SetReadCallback(read_callback_);
  connection->SetWriteCallback(write_callback_);
  connection->SetCloseCallback(std::bind(
    &TcpServer::OnCloseConnectionCallback,
    this,
    fd));
  connection->Init();
  {
    std::lock_guard<std::mutex> lock(mutex_connections_);
    connections_.insert(std::pair<int, ConnectionPtr>(fd, connection));
    LOGF_INFO("New connection from client %s:%u. fd = %d. Current client number: %u",
              addr.ip().c_str(), addr.port(), fd, connections_.size());
  }
  // action on connection
  if(connection_callback_) {
    connection_callback_(connection);
  }
}

void TcpServer::OnCloseConnectionCallback(int fd) {
  LOGF_INFO("Socket closed: %d", fd);
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

EventLoopThreadPoolPtr TcpServer::loop_threads() const {
  return loop_threads_;
}

} // namespace ladder
