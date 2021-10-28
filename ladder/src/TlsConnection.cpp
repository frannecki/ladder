#include <Buffer.h>
#include <TlsConnection.h>

#include <Buffer.h>
#include <Channel.h>
#include <Socket.h>
#include <utils.h>

namespace ladder {

const uint32_t kMaxBufferSize = 1024;

Ssl::Ssl(SSL_CTX* ctx, bool server)
    : ssl_(SSL_new(ctx)),
      rbio_(BIO_new(BIO_s_mem())),
      wbio_(BIO_new(BIO_s_mem())) {
  if (server)
    SSL_set_accept_state(ssl_);
  else
    SSL_set_connect_state(ssl_);
  SSL_set_bio(ssl_, rbio_, wbio_);
}

Ssl::~Ssl() {
  SSL_free(ssl_);
  ssl_ = nullptr;
  rbio_ = wbio_ = nullptr;
}

TlsConnection::TlsConnection(const EventLoopPtr& loop, int fd, SSL_CTX* ssl_ctx,
                             bool server)
    : Connection(loop, fd, false),
      ssl_engine_(new Ssl(ssl_ctx, server)),
      accepted_(false),
      is_server_(server),
      pre_write_buffer_(new Buffer) {}

TlsConnection::~TlsConnection() {
  if (pre_write_buffer_) delete pre_write_buffer_;
  if (ssl_engine_) delete ssl_engine_;
}

void TlsConnection::Init() {
  Connection::Init();
  if (!is_server_) {
    // ssl clients need to send client hello on connection
#ifdef __linux__
    channel_->SetEpollEdgeTriggered(false);
#endif
    channel_->EnableWrite();
  }
}

void TlsConnection::Send(const std::string& buf) {
  if (shut_down_) return;
  if (!SSL_is_init_finished(ssl_engine_->ssl_)) {
    pre_write_buffer_->Write(buf);
    return;
  }
  // encrypt all data
  int len = buf.size();
  int n_written = 0;
  while (n_written < len) {
    int n =
        SSL_write(ssl_engine_->ssl_, buf.c_str() + n_written, len - n_written);
    if (n > 0) {
      n_written += n;
      this->Encrypt();
    } else {
      break;
    }
  }

  EnableWrite(Connection::WriteBuffer());
}

int TlsConnection::ReadBuffer() {
  Buffer buffer;
  int ret = buffer.ReadBufferFromFd(channel_->fd());
  std::string str = buffer.ReadAll();
  if (ReadFromBuffer(str.c_str(), str.size()) < 0) {
    return -2;
  }
  return ret;
}

int TlsConnection::WriteBuffer() {
  if (!is_server_) {
    // client hello shoube be sent once connection is established
    if (!SSL_is_init_finished(ssl_engine_->ssl_)) {
      int n_ssl_connect = SSL_do_handshake(ssl_engine_->ssl_);
      if (n_ssl_connect > 0)
        OnSslHandshakeFinished();
      else
        HandleSslError(n_ssl_connect);
    }
  }
  return Connection::WriteBuffer();
}

void TlsConnection::Encrypt() {
  char ssl_buffer[kMaxBufferSize];
  while (1) {
    int n_ssl_read = BIO_read(ssl_engine_->wbio_, ssl_buffer, kMaxBufferSize);
    if (n_ssl_read > 0) {
      write_buffer_->Write(std::string(ssl_buffer, n_ssl_read));
    } else {
      break;
    }
  }
}

int TlsConnection::Decrypt() {
  char ssl_buffer[kMaxBufferSize];
  int n_read;
  while (1) {
    n_read = SSL_read(ssl_engine_->ssl_, ssl_buffer, kMaxBufferSize);
    if (n_read <= 0) break;
    read_buffer_->Write(std::string(ssl_buffer, n_read));
  }
  return n_read;
}

int TlsConnection::ReadFromBuffer(const char* src, int len) {
  if (len <= 0) return 0;

  int cur = 0;
  while (cur < len) {
    int n = BIO_write(ssl_engine_->rbio_, src + cur, len - cur);
    if (n <= 0) {
      EXIT("BIO_write.");
    }
    cur += n;

    if (!SSL_is_init_finished(ssl_engine_->ssl_)) {
      int n_accepted = SSL_do_handshake(ssl_engine_->ssl_);
      if (n_accepted > 0)
        OnSslHandshakeFinished();

      if (HandleSslError(n_accepted) < 0) {
        return -1;
      }

      if (!SSL_is_init_finished(ssl_engine_->ssl_)) {
        continue;
      }
    }

    int n_read = Decrypt();
    if (HandleSslError(n_read) < 0) {
      return -1;
    }
  }
  return 0;
}

int TlsConnection::HandleSslError(int n) {
  if (n > 0) return 1;
  int err = SSL_get_error(ssl_engine_->ssl_, n);
  switch (err) {
    case SSL_ERROR_NONE:
      break;
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_READ:
      this->Encrypt();
      EnableWrite(Connection::WriteBuffer());
      break;
    default:
      return -1;
  }
  return 0;
}

void TlsConnection::OnSslHandshakeFinished() {
  std::string buf = pre_write_buffer_->ReadAll();
  Send(buf);
}

}  // namespace ladder
