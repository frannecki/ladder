#include <Buffer.h>
#include <Connection.h>

#include "HttpServer.h"

using namespace std::placeholders;

namespace ladder {
namespace http {

HttpServer::HttpServer(const SocketAddr& addr, const char* cert_path,
                       const char* key_path)
    : TcpServer(addr, true, cert_path, key_path),
      codec_(new HttpCodec(cert_path == nullptr || key_path == nullptr)) {
  codec_->set_client_message_callback(
      std::bind(&HttpServer::OnMessage, this, _1, _2));
  SetReadCallback(std::bind(&HttpCodec::OnClientMessage, codec_, _1, _2));
}

HttpServer::~HttpServer() { delete codec_; }

void HttpServer::OnMessage(struct HttpContext* ctx1, struct HttpContext* ctx2) {
  ctx2->clear();
  ctx2->version_ = ctx1->version_;
  auto iter =
      callbacks_.find(static_cast<enum kHttpRequestMethod>(ctx1->method_));
  if (iter == callbacks_.end()) {
    ctx2->status_code_ = kHttpStatusCode::kNotImplemented;
  } else {
    iter->second(ctx1, ctx2);
  }
}

void HttpServer::RegisterCallback(enum kHttpRequestMethod method,
                                  const RequestCallback& callback) {
  callbacks_[method] = callback;
}

}  // namespace http
}  // namespace ladder
