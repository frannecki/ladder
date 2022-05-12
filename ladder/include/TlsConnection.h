#ifndef LADDER_TLS_CONNECTION_H
#define LADDER_TLS_CONNECTION_H

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "Connection.h"

namespace ladder {

struct Ssl {
 public:
  Ssl(SSL_CTX* ctx, bool server = true);
  Ssl(const Ssl&) = delete;
  Ssl& operator=(const Ssl&) = delete;
  ~Ssl();

  SSL* ssl_;
  BIO* rbio_;
  BIO* wbio_;
};

class TlsConnection : public Connection {
 public:
#ifdef LADDER_OS_WINDOWS
  TlsConnection(int fd, SSL_CTX* ssl_cxt, bool server = true);
  void Init(HANDLE iocp_port, char* buffer, int io_size) override;
#else
  TlsConnection(const EventLoopPtr& loop, int fd, SSL_CTX* ssl_cxt,
                bool server = true);
  void Init() override;
#endif
  ~TlsConnection() override;
  void Send(const std::string& buf) override;

 private:
#ifdef LADDER_OS_WINDOWS
  int ReadBuffer(int io_size) override;
  int WriteBuffer(int io_size) override;
#else
  int ReadBuffer() override;
  int WriteBuffer() override;
#endif

  void Encrypt();
  int Decrypt();
  int ReadFromBuffer(const char* src, int len);
  int HandleSslError(int n);
  void OnSslHandshakeFinished();

  Ssl* ssl_engine_;
  bool accepted_;
  bool is_server_;

  // store data sent by application before ssl connection established
  Buffer* pre_write_buffer_;
};

}  // namespace ladder

#endif
