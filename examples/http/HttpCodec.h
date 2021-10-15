#ifndef LADDER_EXAMPLE_HTTP_CODEC_H
#define LADDER_EXAMPLE_HTTP_CODEC_H

#include <map>
#include <string>

#include <Base.h>

namespace ladder {
namespace http {

struct HttpContext {
  int method_;
  std::string uri_;
  std::string version_;
  std::string start_line_;
  std::map<int, std::string> headers_;
  std::string payload_;

  int status_code_;
  int content_encoding_;
  int content_length_;

  void clear();
};

class HttpMessage {
 public:
  HttpMessage();
  virtual ~HttpMessage();
  bool ComposeHeaders(std::string& message);
  int ParseMessage(const std::string& message, int& length);
  struct HttpContext* context();

  virtual int HandleMessage() = 0;
  virtual bool PrepareMessage() = 0;

 protected:
  struct HttpContext* context_;
};

class HttpRequest : public HttpMessage {
 public:
  HttpRequest();
  int HandleMessage() override;
  bool PrepareMessage() override;
};

class HttpResponse : public HttpMessage {
 public:
  HttpResponse();
  int HandleMessage() override;
  bool PrepareMessage() override;
};

class HttpCodec {
  using HttpCodecMessageCallback =
      std::function<void(struct HttpContext*, struct HttpContext*)>;

 public:
  HttpCodec();
  ~HttpCodec();
  void OnClientMessage(const ConnectionPtr& conn, Buffer* buffer);
  void SetClientMessageCallback(const HttpCodecMessageCallback& callback);
  void ParseRequest();
  void ParseResponse();

 private:
  HttpMessage* request_;
  HttpMessage* response_;
  HttpCodecMessageCallback server_callback_;
};

}  // namespace http
}  // namespace ladder

#endif
