#ifdef __unix__
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <fcntl.h>

#include <Acceptor.h>
#include <Base.h>
#include <Channel.h>
#include <Connection.h>
#include <EventLoop.h>
#include <EventLoopThreadPool.h>
#include <Logging.h>
#include <TcpServer.h>
#include <ThreadPool.h>
#include <TlsConnection.h>

using namespace std::placeholders;

namespace ladder {
#ifdef __unix__
void signal_handler(int signum) {
  if (signum == SIGPIPE) {
    LOG_FATAL("SIGPIPE ignored");
  }
}
#endif

using TcpConnectionCloseCallback =
    std::unique_ptr<std::function<void(TcpServer*, int)>>;

TcpServer::TcpServer(const SocketAddr& addr, bool send_file,
                     const char* cert_path, const char* key_path,
                     size_t loop_thread_num, size_t working_thread_num)
    : addr_(addr),
      send_file_(send_file),
      loop_thread_num_(loop_thread_num),
      working_thread_num_(working_thread_num),
      ssl_ctx_(nullptr) {
#ifdef __unix__
  signal(SIGPIPE, signal_handler);
#endif
  if (cert_path != nullptr && key_path != nullptr) {
    ssl_ctx_ = CreateSslContext(true);
    ConfigureSslContext(ssl_ctx_, cert_path, key_path);
    send_file_ = false;
  }
}

TcpServer::~TcpServer() {
  socket::close(channel_->fd());
  channel_->RemoveFromLoop();
  if (ssl_ctx_) {
    SSL_CTX_free(ssl_ctx_);
    ssl_ctx_ = nullptr;
  }
}

void TcpServer::Start() {
  loop_ = std::make_shared<EventLoop>();
  working_threads_ = std::make_shared<ThreadPool>(working_thread_num_);
  int fd = socket::socket(true, addr_.ipv6());
  addr_.Bind(fd);
  socket::listen(fd);
  channel_.reset(new Channel(loop_, fd));
  channel_->UpdateToLoop();
  acceptor_.reset(new Acceptor(channel_, addr_.ipv6()));
  acceptor_->set_new_connection_callback(
      std::bind(&TcpServer::OnNewConnectionCallback, this,
                std::placeholders::_1, std::placeholders::_2));
  LOGF_INFO("Listening on fd = %d %s:%u", fd, addr_.ip().c_str(), addr_.port());
  loop_threads_.reset(new EventLoopThreadPool(loop_thread_num_));
  loop_->StartLoop();
}

void TcpServer::set_read_callback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void TcpServer::set_write_callback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void TcpServer::set_connection_callback(const ConnectionEvtCallback& callback) {
  connection_callback_ = callback;
}

void TcpServer::OnNewConnectionCallback(int fd, SocketAddr&& addr) {
  working_threads_->emplace(
      std::bind(&TcpServer::OnNewConnection, this, fd, addr));
}

void TcpServer::OnNewConnection(int fd, const SocketAddr& addr) {
  EventLoopPtr loop = loop_threads_->GetNextLoop();
  ConnectionPtr connection;
  if (ssl_ctx_)
    connection = std::make_shared<TlsConnection>(loop, fd, ssl_ctx_, true);
  else
    connection = std::make_shared<Connection>(loop, fd, send_file_);
  connection->set_read_callback(read_callback_);
  connection->set_write_callback(write_callback_);
  connection->set_close_callback(
      std::bind(&TcpServer::OnCloseConnectionCallback, this, fd));
  connection->Init();
  {
    std::lock_guard<std::mutex> lock(mutex_connections_);
    connections_.insert(std::pair<int, ConnectionPtr>(fd, connection));
    LOGF_INFO(
        "New connection from client %s:%u. fd = %d. Current client number: %u",
        addr.ip().c_str(), addr.port(), fd, connections_.size());
  }
  // action on connection
  if (connection_callback_) {
    connection_callback_(connection);
  }
}

void TcpServer::OnCloseConnectionCallback(int fd) {
  LOGF_INFO("Socket closed: %d", fd);
  {
    std::lock_guard<std::mutex> lock(mutex_connections_);
    auto iter = connections_.find(fd);
    if (iter != connections_.end()) {
      connections_.erase(iter);
    }
  }
}

EventLoopPtr TcpServer::loop() const { return loop_; }

EventLoopThreadPoolPtr TcpServer::loop_threads() const { return loop_threads_; }

}  // namespace ladder
