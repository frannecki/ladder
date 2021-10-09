#include <Connection.h>
#include <Buffer.h>
#include <Logging.h>

#include "HttpServer.h"

using namespace std::placeholders;

namespace ladder {
namespace http {

HttpServer::HttpServer(const SocketAddr& addr) : 
  TcpServer(addr, true),
  codec_(new HttpCodec)
{
  codec_->SetClientMessageCallback(std::bind(
    &HttpServer::OnMessage, this, _1, _2));
  SetReadCallback(std::bind(&HttpCodec::OnClientMessage,
                            codec_, _1, _2));
}

void HttpServer::OnMessage(struct HttpContext* ctx1,
                           struct HttpContext* ctx2)
{
  ctx2->clear();
  auto iter = callbacks_.find(
    static_cast<enum kHttpRequestMethod>(ctx1->method_));
  if(iter == callbacks_.end()) {
    ctx2->status_code_ = kHttpStatusCode::kNotImplemented;
  }
  else {
    iter->second(ctx1, ctx2);
  }
}

void HttpServer::RegisterCallback(enum kHttpRequestMethod method,
                                  const RequestCallback& callback)
{
  callbacks_[method] = callback;
}

} // namespace http
} // namespace ladder
