#include <unordered_map>

#include <Logging.h>
#include <utils.h>

#include "HttpServer.h"
#include "HttpCodec.h"

using namespace ladder;
using namespace ladder::http;

static const std::string kUnknownFileMimeType = "text/plain; charset=utf-8";

static const std::unordered_map<std::string, std::string> kFileMimeTypes = \
{
  {"htm", "text/html; charset=utf-8"}, {"html", "text/html; charset=utf-8"},
  {"ico", "image/vnd.microsoft.icon"}, {"js", "text/javascript; charset=utf-8"},
  {"css", "text/css"}, {"csv", "text/csv; charset=utf-8"}, {".txt", "text/plain; charset=utf-8"},
  {"jpg", "image/jpeg"}, {"json", "application/json; charset=utf-8"}, {"jpeg", "image/jpeg"},
  {"png", "image/png"}, {"tif", "image/tiff"}, {"tiff", "image/tiff"}, {"pdf", "application/pdf"},
  {"svg", "image/svg+xml; charset=utf-8"}, {"bmp", "image/bmp"}, {"avi", "video/x-msvideo"},
  {"epub", "application/epub+zip"}, {"gif", "application/epub+zip"}, {"mp3", "audio/mpeg"},
  {"mp4", "video"}, {"xml", "application/xml; charset=utf-8"}
};

static std::string GetFileSuffix(const std::string& filename) {
  std::string suffix;
  for(int i = filename.size()-1; i >= 0; --i) {
    if(filename[i] == '.') {
      suffix = filename.substr(i+1, filename.size()-(i+1));
      break;
    }
  }
  return suffix;
}

static std::string kResourceRoot = "resources/";

void HandleHttpGet(struct HttpContext* ctx1, struct HttpContext* ctx2) {
  if(ctx1->status_code_ == kHttpStatusCode::kOk) {

    if(ctx1->uri_.empty()) {
      ctx1->uri_ = "index.html";
    }

    auto iter = kFileMimeTypes.find(GetFileSuffix(ctx1->uri_));
    if(iter != kFileMimeTypes.end()) {
      ctx2->headers_[kHttpHeaderField::kContentType] = iter->second;
    }
    else {
      ctx2->headers_[kHttpHeaderField::kContentType] = kUnknownFileMimeType;
    }

    if(CheckIfFileExists(kResourceRoot + ctx1->uri_)) {
      ctx2->uri_ = kResourceRoot + ctx1->uri_;
      ctx2->status_code_ = kHttpStatusCode::kOk;
      ctx2->headers_[kHttpHeaderField::kContentLength] = std::to_string(GetFileSize(ctx2->uri_));
    }
    else {
      ctx2->headers_[kHttpHeaderField::kContentLength] = "0";
      ctx2->status_code_ = kHttpStatusCode::kNotFound;
    }
  }
}

int main(int argc, char** argv) {
  if(argc > 1) {
    kResourceRoot = std::string(argv[1]);
  }
  Logger::create("./test_http_server.log");
  SocketAddr addr("0.0.0.0", 8070, false);
  HttpServer server(addr);
  server.RegisterCallback(kHttpRequestMethod::kHttpGet, HandleHttpGet);
  server.Start();
  Logger::release();
  return 0;
}
