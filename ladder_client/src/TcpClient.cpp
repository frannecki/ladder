#include <TcpClient.h>
#include <Socket.h>
#include <Channel.h>
#include <Connection.h>
#include <Connector.h>
#include <Timer.h>

#include <Logger.h>

namespace ladder {

TcpClient::TcpClient(const SocketAddr& target_addr,
                     const EventLoopPtr& loop,
                     uint16_t retry_initial_timeout,
                     int max_retry) : 
  max_retry_(max_retry),
  status_(TcpConnectionStatus::kDisconnected),
  target_addr_(target_addr), loop_(loop),
  retry_initial_timeout_(retry_initial_timeout)
{

}

TcpClient::~TcpClient() {
  Disconnect();
  LOG_DEBUG("Releasing tcp client...");
}

void TcpClient::Connect() {
  {
    std::lock_guard<std::mutex> lock(mutex_status_);
    if(status_ != TcpConnectionStatus::kDisconnected) {
      LOG_WARNING("Already connected");
      return;
    }
    status_ = TcpConnectionStatus::kConnecting;
  }
  int fd = socket::socket(true, target_addr_.ipv6());
  conn_.reset(new Connection(loop_, fd));
  conn_->SetReadCallback(read_callback_);
  conn_->SetCloseCallback(std::bind(
    &TcpClient::OnCloseConnectionCallback,
    this,
    fd));
  connector_.reset(new Connector(conn_->channel(),
                                 max_retry_,
                                 target_addr_,
                                 retry_initial_timeout_));
  connector_->SetConnectionCallback(
    std::bind(&TcpClient::OnConnectionCallback, 
              this,
              std::placeholders::_1));
  LOG_INFO("Trying to establish connection to target. fd = " + std::to_string(fd));
  conn_->Init();
  
  // Try connecting
  connector_->Start();
}

void TcpClient::Disconnect() {
  {
    std::lock_guard<std::mutex> lock(mutex_status_);
    if(status_ != TcpConnectionStatus::kConnected) {
      return;
    }
    status_ = TcpConnectionStatus::kConnecting;
  }
  conn_->channel()->ShutDownWrite();
}

void TcpClient::SetReadCallback(const ReadEvtCallback& callback) {
  read_callback_ = callback;
}

void TcpClient::SetWriteCallback(const WriteEvtCallback& callback) {
  write_callback_ = callback;
}

void TcpClient::SetConnectionCallback(const ConnectionEvtCallback& callback) {
  connection_callback_ = callback;
}

void TcpClient::OnConnectionFailureCallback() {
  LOG_INFO("Failed to connect to remote server. fd = " + \
            std::to_string(conn_->channel()->fd()));
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
  LOG_INFO("Connected to remote server " + target_addr_.ip() + \
           ":" + std::to_string(target_addr_.port()) + " from " + \
           addr.ip() + ":" + std::to_string(addr.port()) + ". fd = " \
           + std::to_string(conn_->channel()->fd()));
  
  conn_->SetWriteCallback(write_callback_);
  if(connection_callback_) {
    connection_callback_(conn_);
  }
}

void TcpClient::OnCloseConnectionCallback(int fd) {
  LOG_DEBUG("Socket closed: " + std::to_string(fd));
}

EventLoopPtr TcpClient::loop() const {
  return loop_;
}

} // namespace ladder
