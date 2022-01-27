#include <vector>

#include <Buffer.h>
#include <Connection.h>
#include <GZip.h>
#include <MemoryPool.h>
#include <utils.h>

#include "HttpCodec.h"
#include "http_defs.h"

namespace ladder {
namespace http {

static std::vector<std::string> SplitString(const std::string& str,
                                            const std::string& breaking) {
  std::vector<int> breakpoints = FindSubstr(str, breaking);

  size_t num_breakpoints = breakpoints.size();
  std::vector<std::string> splits;

  int cur_start = 0;
  for (size_t i = 0; i < num_breakpoints; ++i) {
    splits.emplace_back(str.substr(cur_start, breakpoints[i] - cur_start));
    cur_start = breakpoints[i] + breaking.size();
  }

  splits.emplace_back(str.substr(cur_start, str.size() - cur_start));

  return splits;
}

static bool EncodingSupported(int encoding) {
  if (encoding == kAcceptContentEncoding::kIdentity ||
      encoding == kAcceptContentEncoding::kGzip) {
    return true;
  }
  return false;
}

void HttpContext::clear() {
  uri_.clear();
  headers_.clear();
  start_line_.clear();
  payload_.clear();
}

HttpMessage::HttpMessage() : context_(new HttpContext) {}

HttpMessage::~HttpMessage() { delete context_; }

struct HttpContext* HttpMessage::context() {
  return context_;
}

int HttpMessage::ParseMessage(const std::string& message, int& length) {
  length = 0;
  std::vector<int> double_breakpoints =
      FindSubstr(message, kHeaderBreak + kHeaderBreak);
  if (double_breakpoints.size() < 1) {
    return -1;  // packet incomplete
  }

  std::vector<std::string> lines =
      SplitString(message.substr(0, double_breakpoints[0]), kHeaderBreak);

  context_->clear();

  for (size_t i = 0; i < lines.size(); ++i) {
    if (i == 0) {
      context_->start_line_ = lines[i];
      continue;
    }

    std::string& line = lines[i];
    int position_colon = -1;
    for (size_t j = 0; j < line.size(); ++j) {
      if (line[j] == ':') {
        position_colon = j;
        break;
      }
    }

    if (position_colon == -1) {
      return kHttpStatusCode::kBadRequest;
    }

    std::string key = line.substr(0, position_colon);
    auto iter = kHeaderFields.find(std::move(key));
    if (iter != kHeaderFields.end()) {
      context_->headers_[iter->second] =
          line.substr(position_colon + 2, line.size() - position_colon - 2);
    }
  }

  int payload_length =
      message.size() - double_breakpoints[0] - 2 * kHeaderBreak.size();

  auto iter = context_->headers_.find(kHttpHeaderField::kContentLength);
  if (iter != context_->headers_.end()) {
    int content_length = std::atoi(iter->second.c_str());
    if (content_length > payload_length) {
      return -1;  // packet incomplete
    } else {
      payload_length = content_length;
    }
  } else {
    payload_length = 0;
  }

  length = double_breakpoints[0] + 2 * kHeaderBreak.size() + payload_length;
  context_->payload_ = message.substr(
      double_breakpoints[0] + 2 * kHeaderBreak.size(), payload_length);

  return HandleMessage();
}

bool HttpMessage::ComposeHeaders(std::string& message) {
  if (!PrepareMessage()) return false;
  message += context_->start_line_ + kHeaderBreak;

  context_->headers_[kHttpHeaderField::kContentLength] =
      std::to_string(context_->content_length_);

  for (auto iter = context_->headers_.begin(); iter != context_->headers_.end();
       ++iter) {
    auto iter_field = kHeaderFieldStrs.find(iter->first);
    if (iter_field != kHeaderFieldStrs.end()) {
      message += iter_field->second + ": " + iter->second + kHeaderBreak;
    }
  }

  message += kHeaderBreak;

  return true;
}

HttpRequest::HttpRequest() {
  context_->version_ = "HTTP/1.1";
  context_->headers_[kHttpHeaderField::kConnection] = "close";
}

int HttpRequest::HandleMessage() {
  std::vector<std::string> http_method_fields =
      SplitString(context_->start_line_, " ");  // method, uri and version

  if (http_method_fields.size() != 3) {
    return kHttpStatusCode::kBadRequest;
  }

  {
    auto iter = kHttpRequestMethods.find(http_method_fields[0]);
    if (iter == kHttpRequestMethods.end()) {
      return kHttpStatusCode::kMethodNotAllowed;
    }
    context_->method_ = iter->second;
  }

  if (http_method_fields[1].empty() || http_method_fields[1].front() != '/') {
    return kHttpStatusCode::kBadRequest;
  }

  context_->uri_ =
      http_method_fields[1].substr(1, http_method_fields[1].size() - 1);
  context_->version_ = http_method_fields[2];

  context_->content_encoding_ = kAcceptContentEncoding::kIdentity;
  auto iter = context_->headers_.find(kHttpHeaderField::kAcceptEncoding);
  if (iter != context_->headers_.end()) {
    std::vector<std::string> accept_encodings = SplitString(iter->second, ", ");
    for (std::string& enc : accept_encodings) {
      auto iter_enc = kContentEncodings.find(enc);
      if (iter_enc != kContentEncodings.end() &&
          EncodingSupported(iter_enc->second)) {
        context_->content_encoding_ = iter_enc->second;
        break;
      }
    }
  }

  return kHttpStatusCode::kOk;
}

bool HttpRequest::PrepareMessage() {
  auto iter = kHttpRequestMethodStrs.find(context_->method_);
  if (iter == kHttpRequestMethodStrs.end()) {
    return false;
  }
  context_->start_line_ =
      iter->second + " " + context_->uri_ + " " + context_->version_;
  return true;
}

HttpResponse::HttpResponse() {
  context_->version_ = "HTTP/1.1";
  context_->headers_[kHttpHeaderField::kConnection] = "close";
}

int HttpResponse::HandleMessage() {
  std::vector<std::string> http_method_fields =
      SplitString(context_->start_line_, " ");  // method, uri and version

  if (http_method_fields.size() != 3) {
    return -1;
  }

  {
    auto iter = kHttpRequestMethods.find(http_method_fields[0]);
    if (iter == kHttpRequestMethods.end()) {
      return -1;
    }
    context_->method_ = iter->second;
  }

  context_->version_ = http_method_fields[0];
  context_->method_ = std::atoi(http_method_fields[1].c_str());

  context_->content_encoding_ = kAcceptContentEncoding::kIdentity;
  auto iter = context_->headers_.find(kHttpHeaderField::kAcceptEncoding);
  if (iter != context_->headers_.end()) {
    std::vector<std::string> accept_encodings = SplitString(iter->second, ", ");
    auto iter_enc = kContentEncodings.find(accept_encodings[0]);
    if (iter_enc != kContentEncodings.end() &&
        EncodingSupported(iter_enc->second)) {
      context_->content_encoding_ = iter_enc->second;
    } else {
      return kHttpStatusCode::kNotAcceptable;
    }
  }

  return 0;
}

bool HttpResponse::PrepareMessage() {
  auto iter = kHttpStatus.find(context_->status_code_);
  if (iter == kHttpStatus.end()) {
    return false;
  }
  context_->start_line_ = context_->version_ + " " + iter->second;

  return true;
}

HttpCodec::HttpCodec(bool send_file)
    : send_file_(send_file) {}

void HttpCodec::SetClientMessageCallback(
    const HttpCodecMessageCallback& callback) {
  server_callback_ = callback;
}

void HttpCodec::OnClientMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message;
  int length = 0;
  buffer->Peek(buffer->ReadableBytes(), message);

  // TODO: implement multi-threaded http server

  HttpMessage* request = new HttpRequest;
  HttpMessage* response = new HttpResponse;

  int status = request->ParseMessage(message, length);
  if (status == -1) {
    // message incomplete;
    return;
  }

  buffer->HaveRead(length);

  request->context()->status_code_ = status;

  if (server_callback_) {
    server_callback_(request->context(), response->context());
  }

  std::string headers;

  if (response->context()->status_code_ == kHttpStatusCode::kOk) {
    if (!send_file_ || request->context()->content_encoding_ == kGzip) {
      std::string filebuf;
      if (request->context()->content_encoding_ == kGzip) {
        filebuf = GZipper::DeflateFile(response->context()->uri_);
        response->context()->headers_[kHttpHeaderField::kContentEncoding] =
            "gzip";
      } else {
        filebuf = ReadFileAsString(response->context()->uri_);
      }
      response->context()->content_length_ = filebuf.size();

      response->ComposeHeaders(headers);
      conn->Send(std::move(headers) + std::move(filebuf));
    } else {
      response->ComposeHeaders(headers);
      conn->SendFile(std::move(headers), response->context()->uri_);
    }
  } else {
    response->ComposeHeaders(headers);
    conn->Send(std::move(headers));
  }

  if (request->context()->version_ == "HTTP/1.0") conn->ShutDownWrite();

  delete request;
  delete response;

}

}  // namespace http
}  // namespace ladder
