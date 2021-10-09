#include <vector>

#include <utils.h>
#include <Connection.h>
#include <Buffer.h>
#include <Logging.h>

#include "HttpCodec.h"
#include "http_defs.h"

namespace ladder {
namespace http {

void HttpContext::clear() {
  uri_.clear();
  headers_.clear();
  start_line_.clear();
  payload_.clear();
}

HttpMessage::HttpMessage() : context_(new HttpContext) {

}

struct HttpContext* HttpMessage::context() {
  return context_;
}

int HttpMessage::ParseMessage(const std::string& message, int &length) {
  length = 0;
  std::vector<int> double_breakpoints = FindSubstr(
    message, kHeaderBreak + kHeaderBreak);
  if(double_breakpoints.size() < 1) {
    return -1;  // packet incomplete
  }
  
  std::vector<int> breakpoints = FindSubstr(
    message.substr(0, double_breakpoints[0] + kHeaderBreak.size()),
    kHeaderBreak);
  
  context_->clear();
  size_t num_breakpoints = breakpoints.size();

  for(size_t i = 0; i < num_breakpoints; ++i) {
    if(i == 0) {
      context_->start_line_ = message.substr(0, breakpoints[i]);
      continue;
    }
    std::string line = message.substr(breakpoints[i-1] + kHeaderBreak.size(),
                                      breakpoints[i] - breakpoints[i-1] - kHeaderBreak.size());
    int position_colon = -1;
    for(size_t j = 0; j < line.size(); ++j) {
      if(line[j] == ':') {
        position_colon = j;
        break;
      }
    }

    if(position_colon == -1) {
      return kHttpStatusCode::kBadRequest;
    }

    std::string key = line.substr(0, position_colon);
    auto iter = kHeaderFields.find(std::move(key));
    if(iter != kHeaderFields.end()) {
      context_->headers_[iter->second] = line.substr(position_colon + 1,
                                                     line.size() - position_colon - 1);
    }
  }

  int payload_length = message.size() - double_breakpoints[0] - 2 * kHeaderBreak.size();

  auto iter = context_->headers_.find(kHttpHeaderField::kContentLength);
  if(iter != context_->headers_.end()) {
    int content_length = std::atoi(iter->second.c_str());
    if(content_length > payload_length) {
      return -1;  // packet incomplete
    }
    else {
      payload_length = content_length;
    }
  }
  else {
    payload_length = 0;
  }

  length = double_breakpoints[0] + 2 * kHeaderBreak.size() + payload_length;
  context_->payload_ = message.substr(double_breakpoints[0] + 2 * kHeaderBreak.size(),
                                      payload_length);

  return HandleMessage();
}

bool HttpMessage::ComposeHeaders(std::string& message) {
  if(!PrepareMessage())  return false;
  message += context_->start_line_ + kHeaderBreak;
  for(auto iter = context_->headers_.begin(); iter != context_->headers_.end(); ++iter) {
    auto iter_field = kHeaderFieldStrs.find(iter->first);
    if(iter_field != kHeaderFieldStrs.end()) {
      message += iter_field->second + ": " + iter->second + kHeaderBreak;
    }
  }
  message += kHeaderBreak;

  return true;
}

HttpRequest::HttpRequest() {
  context_->version_ = "HTTP/1.1";
}

int HttpRequest::HandleMessage() {
  size_t cur_start = 0;
  std::vector<std::string> http_method_fields;  // method, uri and version
  for(size_t j = 0; j <= context_->start_line_.size(); ++j) {
    if(context_->start_line_[j] == ' ' || j == context_->start_line_.size()) {
      http_method_fields.push_back(
        context_->start_line_.substr(cur_start, j - cur_start));
      cur_start = j + 1;
    }
  }
  
  if(http_method_fields.size() != 3) {
    return kHttpStatusCode::kBadRequest;
  }

  {
    auto iter = kHttpRequestMethods.find(http_method_fields[0]);
    if(iter == kHttpRequestMethods.end()) {
      return kHttpStatusCode::kMethodNotAllowed;
    }
    context_->method_ = iter->second;
  }

  if(http_method_fields[1].empty() || http_method_fields[1].front() != '/') {
    return kHttpStatusCode::kBadRequest;
  }

  context_->uri_ = http_method_fields[1].substr(
    1, http_method_fields[1].size() - 1);
  context_->version_ = http_method_fields[2];

  return kHttpStatusCode::kOk;
}

bool HttpRequest::PrepareMessage() {
  auto iter = kHttpRequestMethodStrs.find(context_->method_);
  if(iter == kHttpRequestMethodStrs.end()) {
    return false;
  }
  context_->start_line_ = iter->second + " " + context_->uri_ + " " + context_->version_;
  return true;
}

HttpResponse::HttpResponse() {
  context_->version_ = "HTTP/1.1";
}

int HttpResponse::HandleMessage() {
  size_t cur_start = 0;
  std::vector<std::string> http_method_fields;  // method, uri and version
  for(size_t j = 0; j <= context_->start_line_.size(); ++j) {
    if(context_->start_line_[j] == ' ' || j == context_->start_line_.size()) {
      http_method_fields.push_back(
        context_->start_line_.substr(cur_start, j - cur_start));
      cur_start = j + 1;
    }
  }
  
  if(http_method_fields.size() != 3) {
    return -1;
  }

  {
    auto iter = kHttpRequestMethods.find(http_method_fields[0]);
    if(iter == kHttpRequestMethods.end()) {
      return -1;
    }
    context_->method_ = iter->second;
  }

  context_->version_ = http_method_fields[0];
  context_->method_ = std::atoi(http_method_fields[1].c_str());

  return 0;
}

bool HttpResponse::PrepareMessage() {
  auto iter = kHttpStatus.find(context_->status_code_);
  if(iter == kHttpStatus.end()) {
    return false;
  }
  context_->start_line_ = context_->version_ + " " + iter->second;

  return true;
}

HttpCodec::HttpCodec() : 
  request_(new HttpRequest),
  response_(new HttpResponse)
{

}

void HttpCodec::SetClientMessageCallback(const HttpCodecMessageCallback& callback) {
  server_callback_ = callback;
}

void HttpCodec::OnClientMessage(const ConnectionPtr& conn, Buffer* buffer) {
  std::string message;
  int length = 0;
  buffer->Peek(buffer->ReadableBytes(), message);
  int status = request_->ParseMessage(message, length);
  if(status == -1) {
    // message incomplete;
    return;
  }
  buffer->HaveRead(length);
  request_->context()->status_code_ = status;

  if(server_callback_) {
    server_callback_(request_->context(),
                     response_->context());
  }

  std::string headers;
  
  response_->ComposeHeaders(headers);
  if(response_->context()->status_code_ == kHttpStatusCode::kOk) {
    conn->SendFile(std::move(headers), response_->context()->uri_);
  }
  else {
    conn->Send(std::move(headers));
  }
}

} // namespace http
} // namespace ladder
