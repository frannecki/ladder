#ifndef LADDER_TLS_CONNECTION_H
#define LADDER_TLS_CONNECTION_H

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <Connection.h>

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
  TlsConnection(const EventLoopPtr& loop, int fd, SSL_CTX* ssl_cxt,
                bool server = true);
  ~TlsConnection() override;
  void Init() override;
  void Send(const std::string& buf) override;

 private:
  int ReadBuffer() override;
  int WriteBuffer() override;

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
