#include <compat.h>
#ifdef LADDER_OS_UNIX
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
#include <utils.h>

#ifdef LADDER_OS_WINDOWS
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace std::placeholders;

namespace ladder {
#ifdef LADDER_OS_UNIX
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
      running_(true),
#ifdef LADDER_OS_WINDOWS
      iocp_port_(NULL),
#else
      working_thread_num_(working_thread_num),
#endif
      ssl_ctx_(nullptr) {
#ifdef LADDER_OS_UNIX
  signal(SIGPIPE, signal_handler);
#endif
  if (cert_path != nullptr && key_path != nullptr) {
    ssl_ctx_ = CreateSslContext(true);
    ConfigureSslContext(ssl_ctx_, cert_path, key_path);
    send_file_ = false;
  }
}

TcpServer::~TcpServer() {
  Stop();
  // This cannot be put in Stop() in case of a call to Stop() in connection callback,
  // which causes the thread pool to be destructed (where the same thread would be joined),
  // causing a deadlock.
  working_threads_.reset();
  if (ssl_ctx_) {
    SSL_CTX_free(ssl_ctx_);
    ssl_ctx_ = nullptr;
  }
}

void TcpServer::Start() {
  int fd = socket::socket(true, addr_.ipv6());
  addr_.Bind(fd);
  socket::listen(fd);
#ifdef LADDER_OS_WINDOWS
  channel_.reset(new Channel(fd));
  iocp_port_ = UpdateIocpPort(NULL, channel_.get());
  loop_ = std::make_shared<EventLoop>(iocp_port_);
  acceptor_.reset(new Acceptor(channel_, addr_.ipv6()));
  acceptor_->SetNewConnectionCallback(
      std::bind(&TcpServer::OnNewConnection,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4));
  loop_threads_.reset(new EventLoopThreadPool(iocp_port_, loop_thread_num_));
  acceptor_->Init();
#else
  working_threads_ = std::make_shared<ThreadPool>(working_thread_num_);
  loop_ = std::make_shared<EventLoop>();
  channel_.reset(new Channel(loop_, fd));
  channel_->UpdateToLoop();
  acceptor_.reset(new Acceptor(channel_, addr_.ipv6()));
  acceptor_->SetNewConnectionCallback(
      std::bind(&TcpServer::OnNewConnectionCallback, this,
                std::placeholders::_1, std::placeholders::_2));
  loop_threads_.reset(new EventLoopThreadPool(loop_thread_num_));
#endif
  LOGF_INFO("Listening on fd = %d %s:%u", fd, addr_.ip().c_str(), addr_.port());
  std::lock_guard<std::mutex> lock(mutex_serving_);
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

void TcpServer::Stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_running_);
    running_ = false;
  }
  acceptor_.reset();
  if (channel_) {
    int fd = channel_->fd();
    channel_.reset();
    socket::close(fd);
  }
  if (loop_) loop_->StopLoop();
#ifdef LADDER_OS_WINDOWS
  if (iocp_port_) CloseHandle(iocp_port_);
#else
  if (loop_) loop_->Wakeup(); // wakeup from blocking poll
#endif
  {
    std::lock_guard<std::mutex> lock(mutex_serving_);
    loop_.reset();
  }
  {
    std::lock_guard<std::mutex> lock(mutex_connections_);
    for (auto iter = connections_.begin(); iter != connections_.end(); ++iter) {
      iter->second->SetCloseCallback(nullptr);
      iter->second->Close();
    }
    connections_.clear();
  }
  LOGF_DEBUG("Server stopped: %p.", this);
}

#ifdef LADDER_OS_WINDOWS
void TcpServer::OnNewConnection(int fd, const SocketAddr& addr, char* buffer, int io_size) {
  {
    std::lock_guard<std::mutex> lock(mutex_running_);
    if (!running_) return;
  }
  ConnectionPtr connection;
  if (ssl_ctx_)
    connection = std::make_shared<TlsConnection>(fd, ssl_ctx_, true);
  else
    connection = std::make_shared<Connection>(fd, send_file_);
#else
void TcpServer::OnNewConnectionCallback(int fd, SocketAddr&& addr) {
  working_threads_->emplace(
      std::bind(&TcpServer::OnNewConnection, this, fd, addr));
}

void TcpServer::OnNewConnection(int fd, const SocketAddr& addr) {
  {
    std::lock_guard<std::mutex> lock(mutex_running_);
    if (!running_) return;
  }
  EventLoopPtr loop = loop_threads_->GetNextLoop();
  ConnectionPtr connection;
  if (ssl_ctx_)
    connection = std::make_shared<TlsConnection>(loop, fd, ssl_ctx_, true);
  else
    connection = std::make_shared<Connection>(loop, fd, send_file_);
#endif
  connection->SetReadCallback(read_callback_);
  connection->SetWriteCallback(write_callback_);
  connection->SetCloseCallback(
      std::bind(&TcpServer::OnCloseConnectionCallback, this, fd));
#ifdef LADDER_OS_WINDOWS
  connection->Init(iocp_port_, buffer, io_size);
#else
  connection->Init();
#endif
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

#ifndef LADDER_OS_WINDOWS
EventLoopPtr TcpServer::loop() const { return loop_; }
#endif

EventLoopThreadPoolPtr TcpServer::loop_threads() const { return loop_threads_; }

}  // namespace ladder
