#ifndef LADDER_EXAMPLE_HTTP_SERVER_H
#define LADDER_EXAMPLE_HTTP_SERVER_H

#include <functional>
#include <map>

#include <TcpServer.h>

#include "HttpCodec.h"
#include "http_defs.h"

namespace ladder {
namespace http {

class HttpCodec;

class HttpServer : public TcpServer {
  using RequestCallback =
      std::function<void(struct HttpContext* ctx1, struct HttpContext* cxt2)>;

 public:
  HttpServer(const SocketAddr& addr);
  ~HttpServer();
  void RegisterCallback(enum kHttpRequestMethod,
                        const RequestCallback& callback);

 private:
  void OnMessage(struct HttpContext* ctx1, struct HttpContext* cxt2);

  HttpCodec* codec_;

  std::map<enum kHttpRequestMethod, RequestCallback> callbacks_;
};

}  // namespace http
}  // namespace ladder

#endif
