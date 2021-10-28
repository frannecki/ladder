#include <Channel.h>
#include <Connection.h>
#include <Connector.h>
#include <Socket.h>
#include <TcpClient.h>
#include <Timer.h>
#include <TlsConnection.h>

#include <Logging.h>

namespace ladder {

TcpClient::TcpClient(const SocketAddr& target_addr, const EventLoopPtr& loop,
                     uint16_t retry_initial_timeout, bool use_ssl,
                     int max_retry)
    : max_retry_(max_retry),
      status_(TcpConnectionStatus::kDisconnected),
      target_addr_(target_addr),
      loop_(loop),
      retry_initial_timeout_(retry_initial_timeout),
      ssl_ctx_(use_ssl ? CreateSslContext(false) : nullptr) {}

TcpClient::~TcpClient() {
  Disconnect();
  LOG_DEBUG("Releasing tcp client...");
  if (ssl_ctx_) {
    SSL_CTX_free(ssl_ctx_);
    ssl_ctx_ = nullptr;
  }
}

void TcpClient::Connect() {
  {
    std::lock_guard<std::mutex> lock(mutex_status_);
    if (status_ != TcpConnectionStatus::kDisconnected) {
      LOG_WARNING("Already connected");
      return;
    }
    status_ = TcpConnectionStatus::kConnecting;
  }
  int fd = socket::socket(true, target_addr_.ipv6());
  if (!ssl_ctx_)
    conn_.reset(new Connection(loop_, fd));
  else
    conn_.reset(new TlsConnection(loop_, fd, ssl_ctx_, false));
  conn_->set_read_callback(read_callback_);
  conn_->set_close_callback(
      std::bind(&TcpClient::OnCloseConnectionCallback, this, fd));
  connector_.reset(new Connector(conn_->channel(), max_retry_, target_addr_,
                                 retry_initial_timeout_));
  connector_->set_connection_callback(
      std::bind(&TcpClient::OnConnectionCallback, this, std::placeholders::_1));
  connector_->set_connection_failure_callback(
      std::bind(&TcpClient::OnConnectionFailureCallback, this));
  LOGF_INFO("Trying to establish connection to target. fd = %d", fd);
  conn_->Init();

  // Try connecting
  connector_->Start();
}

void TcpClient::Disconnect() {
  {
    std::lock_guard<std::mutex> lock(mutex_status_);
    if (status_ != TcpConnectionStatus::kConnected) {
      return;
    }
    status_ = TcpConnectionStatus::kConnecting;
  }
  conn_->channel()->ShutDownWrite();
}

void TcpClient::set_read_callback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void TcpClient::set_write_callback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void TcpClient::set_connection_callback(const ConnectionEvtCallback& callback) {
  connection_callback_ = callback;
}

void TcpClient::OnConnectionFailureCallback() {
  LOGF_INFO("Failed to connect to remote server. fd = %d",
            conn_->channel()->fd());
  conn_.reset();
  {
    std::lock_guard<std::mutex> lock(mutex_status_);
    status_ = TcpConnectionStatus::kDisconnected;
  }
}

void TcpClient::OnConnectionCallback(SocketAddr&& addr) {
  {
    std::lock_guard<std::mutex> lock(mutex_status_);
    status_ = TcpConnectionStatus::kConnected;
  }
  LOGF_INFO("Connected to remote server %s:%u from %s:%u. fd = %d",
            target_addr_.ip().c_str(), target_addr_.port(), addr.ip().c_str(),
            addr.port(), conn_->channel()->fd());

  conn_->SetChannelCallbacks();

  if (connection_callback_) {
    connection_callback_(conn_);
  }
}

void TcpClient::OnCloseConnectionCallback(int fd) {
  LOG_DEBUG("Socket closed: " + std::to_string(fd));
}

EventLoopPtr TcpClient::loop() const { return loop_; }

}  // namespace ladder
