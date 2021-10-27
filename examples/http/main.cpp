#include <unordered_map>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <Logging.h>
#include <utils.h>

#include "HttpCodec.h"
#include "HttpServer.h"

using namespace ladder;
using namespace ladder::http;

static const std::string kUnknownFileMimeType = "application/octet-stream";
static const int kCompressibleFileSizeThreshold = 2048;

enum kFileFormat {
  kFileFormatJpg = 0,
  kFileFormatPng,
  kFileFormatIco,
  kFileFormatGif,
  kFileFormatTif,
  kFileFormatPdf,
  kFileFormatEpub,
  kFileFormatMp3,
  kFileFormatMp4,
  kFileFormatAvi,
  kFileFormatZip,

  kFileFormatCompressible,

  kFileFormatHtml,
  kFileFormatJs,
  kFileFormatCss,
  kFileFormatSvg,
  kFileFormatXml,
  kFileFormatText,
  kFileFormatJson,
  kFileFormatCsv,
  kFileFormatBmp,
};

static const std::unordered_map<std::string, int> kFileMimeTypeCodes = {
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
    {"xml", kFileFormat::kFileFormatXml},
    {"zip", kFileFormat::kFileFormatZip}};

static const std::unordered_map<int, std::string> kFileMimeTypes = {
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
    {kFileFormat::kFileFormatXml, "application/xml; charset=utf-8"}};

static int GetFileSuffix(const std::string& filename) {
  std::string suffix;
  for (int i = filename.size() - 1; i >= 0; --i) {
    if (filename[i] == '.') {
      suffix = filename.substr(i + 1, filename.size() - (i + 1));
      break;
    }
  }
  auto iter = kFileMimeTypeCodes.find(suffix);
  if (iter != kFileMimeTypeCodes.end()) {
    return iter->second;
  }
  return -1;
}

static std::string kResourceRoot = "resources/";

void HandleHttpGet(struct HttpContext* ctx1, struct HttpContext* ctx2) {
  if (ctx1->status_code_ == kHttpStatusCode::kOk) {
    if (ctx1->uri_.empty()) {
      ctx1->uri_ = "index.html";
    }

    int fmt = GetFileSuffix(ctx1->uri_);
    auto iter = kFileMimeTypes.find(fmt);
    if (iter != kFileMimeTypes.end()) {
      ctx2->headers_[kHttpHeaderField::kContentType] = iter->second;
    } else {
      ctx2->headers_[kHttpHeaderField::kContentType] = kUnknownFileMimeType;
    }
    if (fmt <= kFileFormat::kFileFormatCompressible) {
      ctx1->content_encoding_ = kAcceptContentEncoding::kIdentity;
    }

    ctx2->uri_ = kResourceRoot + ctx1->uri_;
    int file_size = GetFileSize(ctx2->uri_);
    if (file_size <= kCompressibleFileSizeThreshold) {
      ctx1->content_encoding_ = kAcceptContentEncoding::kIdentity;
    }

    if (file_size >= 0) {
      ctx2->status_code_ = kHttpStatusCode::kOk;
      ctx2->content_length_ = file_size;
    } else {
      ctx2->content_length_ = 0;
      ctx2->status_code_ = kHttpStatusCode::kNotFound;
    }
  }
}

static SSL_CTX* create_context() {
  const SSL_METHOD* method;
  SSL_CTX* ctx;

  method = SSLv23_server_method();

  ctx = SSL_CTX_new(method);
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  return ctx;
}

void configure_context(SSL_CTX* ctx, const std::string& ssl_cert_dir) {
  SSL_CTX_set_ecdh_auto(ctx, 1);

  /* Set the key and cert */
  if (SSL_CTX_use_certificate_file(ctx,
                                   (ssl_cert_dir + "/" + "cert.pem").c_str(),
                                   SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, (ssl_cert_dir + "/" + "key.pem").c_str(),
                                  SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s $ResourceRoot[ $CertPath $KeyPath]\n", argv[0]);
    return EXIT_FAILURE;
  }
  kResourceRoot = std::string(argv[1]);
  std::string ssl_cert_dir = "ssl";
  if (argc > 2) {
    ssl_cert_dir = std::string(argv[2]);
  }
  Logger::create("./test_http_server.log");
  SocketAddr addr("0.0.0.0", 8070, false);

  SslInit();
  HttpServer server(addr, (argc > 2) ? argv[2] : nullptr,
                    (argc > 3) ? argv[3] : nullptr);
  server.RegisterCallback(kHttpRequestMethod::kHttpGet, HandleHttpGet);
  server.Start();
  Logger::release();
  EVP_cleanup();
  return EXIT_SUCCESS;
}
