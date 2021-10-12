#include <unordered_map>

#include <Logging.h>
#include <utils.h>

#include "HttpServer.h"
#include "HttpCodec.h"

using namespace ladder;
using namespace ladder::http;

static const std::string kUnknownFileMimeType = "text/plain; charset=utf-8";

static const int kCompressibleFileSizeThreshold = 2048;

enum kFileFormat {
  kFileFormatHtml = 0,
  kFileFormatJs,
  kFileFormatCss,
  kFileFormatSvg,
  kFileFormatXml,
  kFileFormatText,
  kFileFormatJson,
  kFileFormatCsv,
  kFileFormatBmp,

  kFileFormatCompressible,

  kFileFormatJpg,
  kFileFormatPng,
  kFileFormatIco,
  kFileFormatGif,
  kFileFormatTif,
  kFileFormatPdf,
  kFileFormatEpub,
  kFileFormatMp3,
  kFileFormatMp4,
  kFileFormatAvi,
};

static const std::unordered_map<std::string, int> kFileMimeTypeCodes = \
{
  {"htm", kFileFormat::kFileFormatHtml},
  {"html", kFileFormat::kFileFormatHtml},
  {"ico", kFileFormat::kFileFormatIco},
  {"js", kFileFormat::kFileFormatJs},
  {"css", kFileFormat::kFileFormatCss},
  {"csv", kFileFormat::kFileFormatCsv},
  {"txt", kFileFormat::kFileFormatText},
  {"jpg", kFileFormat::kFileFormatJpg},
  {"jpeg", kFileFormat::kFileFormatJpg},
  {"json", kFileFormat::kFileFormatJson},
  {"png", kFileFormat::kFileFormatPng},
  {"tif", kFileFormat::kFileFormatTif},
  {"tiff", kFileFormat::kFileFormatTif},
  {"pdf", kFileFormat::kFileFormatPdf},
  {"svg", kFileFormat::kFileFormatSvg},
  {"bmp", kFileFormat::kFileFormatBmp},
  {"avi", kFileFormat::kFileFormatAvi},
  {"epub", kFileFormat::kFileFormatEpub},
  {"gif", kFileFormat::kFileFormatGif},
  {"mp3", kFileFormat::kFileFormatMp3},
  {"mp4", kFileFormat::kFileFormatMp4},
  {"xml", kFileFormat::kFileFormatXml}
};

static const std::unordered_map<int, std::string> kFileMimeTypes = \
{
  {kFileFormat::kFileFormatHtml, "text/html; charset=utf-8"},
  {kFileFormat::kFileFormatIco, "image/vnd.microsoft.icon"},
  {kFileFormat::kFileFormatJs, "text/javascript; charset=utf-8"},
  {kFileFormat::kFileFormatCss, "text/css"},
  {kFileFormat::kFileFormatCsv, "text/csv; charset=utf-8"},
  {kFileFormat::kFileFormatText, "text/plain; charset=utf-8"},
  {kFileFormat::kFileFormatJpg, "image/jpeg"},
  {kFileFormat::kFileFormatJson, "application/json; charset=utf-8"},
  {kFileFormat::kFileFormatPng, "image/png"},
  {kFileFormat::kFileFormatTif, "image/tiff"},
  {kFileFormat::kFileFormatPdf, "application/pdf"},
  {kFileFormat::kFileFormatSvg, "image/svg+xml; charset=utf-8"},
  {kFileFormat::kFileFormatBmp, "image/bmp"},
  {kFileFormat::kFileFormatAvi, "video/x-msvideo"},
  {kFileFormat::kFileFormatEpub, "application/epub+zip"},
  {kFileFormat::kFileFormatGif, "image/gif"},
  {kFileFormat::kFileFormatMp3, "audio/mpeg"},
  {kFileFormat::kFileFormatMp4, "video"},
  {kFileFormat::kFileFormatXml, "application/xml; charset=utf-8"}
};

static int GetFileSuffix(const std::string& filename) {
  std::string suffix;
  for(int i = filename.size()-1; i >= 0; --i) {
    if(filename[i] == '.') {
      suffix = filename.substr(i+1, filename.size() - (i+1));
      break;
    }
  }
  auto iter = kFileMimeTypeCodes.find(suffix);
  if(iter != kFileMimeTypeCodes.end()) {
    return iter->second;
  }
  return -1;
}

static std::string kResourceRoot = "resources/";

void HandleHttpGet(struct HttpContext* ctx1, struct HttpContext* ctx2) {
  if(ctx1->status_code_ == kHttpStatusCode::kOk) {

    if(ctx1->uri_.empty()) {
      ctx1->uri_ = "index.html";
    }

    int fmt = GetFileSuffix(ctx1->uri_);
    auto iter = kFileMimeTypes.find(fmt);
    if(iter != kFileMimeTypes.end()) {
      ctx2->headers_[kHttpHeaderField::kContentType] = iter->second;
    }
    else {
      ctx2->headers_[kHttpHeaderField::kContentType] = kUnknownFileMimeType;
    }
    if(fmt >= kFileFormat::kFileFormatCompressible) {
      ctx1->content_encoding_ = kAcceptContentEncoding::kIdentity;
    }

    ctx2->uri_ = kResourceRoot + ctx1->uri_;
    int file_size = GetFileSize(ctx2->uri_);
    if(file_size <= kCompressibleFileSizeThreshold) {
      ctx1->content_encoding_ = kAcceptContentEncoding::kIdentity;
    }

    if(file_size >= 0) {
      ctx2->status_code_ = kHttpStatusCode::kOk;
      ctx2->content_length_ = file_size;
    }
    else {
      ctx2->content_length_ = 0;
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
