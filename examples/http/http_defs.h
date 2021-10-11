#include <string>
#include <unordered_map>

namespace ladder {
namespace http {

enum kHttpRequestMethod : int {
  kHttpGet = 0,
  kHttpHead,
  kHttpPost,
  kHttpPut,
  kHttpDelete,
  kHttpConnect,
  kHttpOptions,
  kHttpTrace,
  kHttpPatch
};

enum kHttpHeaderField : int {
  kAccept = 0,
  kContentType,
  kUserAgent,
  kContentLength,
  kContentEncoding,
  kHost,
  kConnection
};

enum kHttpStatusCode : int {
  kOk = 200,

  kBadRequest = 400,
  kUnauthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kMethodNotAllowed = 405,
  kNotAcceptable = 406,
  kPayloadTooLarge = 413,
  kUriTooLong = 414,

  kInternalServerError = 500,
  kNotImplemented = 501,
  kHttpVersionNotSupported = 505,

};

static const std::string kHeaderBreak = "\r\n";

static const std::unordered_map<int, std::string> kHttpStatus = \
{
  {kHttpStatusCode::kOk, "200 OK"},
  
  {kHttpStatusCode::kBadRequest, "400 Bad Request"},
  {kHttpStatusCode::kUnauthorized, "401 Unauthorized"},
  {kHttpStatusCode::kForbidden, "403 Forbidden"},
  {kHttpStatusCode::kNotFound, "404 Not Found"},
  {kHttpStatusCode::kMethodNotAllowed, "405 Method Not Allowed"},
  {kHttpStatusCode::kNotAcceptable, "406 Not Acceptable"},
  {kHttpStatusCode::kPayloadTooLarge, "413 Payload Too Large"},
  {kHttpStatusCode::kUriTooLong, "414 URI Too Long"},

  {kHttpStatusCode::kInternalServerError, "500 Internal Server Error"},
  {kHttpStatusCode::kNotImplemented, "501 Not Implemented"},
  {kHttpStatusCode::kHttpVersionNotSupported, "505 HTTP Version Not Supported"}
};

static const std::unordered_map<std::string, int> kHttpRequestMethods = \
{
  {"GET", kHttpRequestMethod::kHttpGet},
  {"HEAD", kHttpRequestMethod::kHttpHead},
  {"POST", kHttpRequestMethod::kHttpPost},
  {"PUT", kHttpRequestMethod::kHttpPut},
  {"DELETE", kHttpRequestMethod::kHttpDelete},
  {"CONNECT", kHttpRequestMethod::kHttpConnect},
  {"OPTIONS", kHttpRequestMethod::kHttpOptions},
  {"TRACE", kHttpRequestMethod::kHttpTrace},
  {"PATCH", kHttpRequestMethod::kHttpPatch}
};

static const std::unordered_map<int, std::string> kHttpRequestMethodStrs = \
{
  {kHttpRequestMethod::kHttpGet, "GET"},
  {kHttpRequestMethod::kHttpHead, "HEAD"},
  {kHttpRequestMethod::kHttpPost, "POST"},
  {kHttpRequestMethod::kHttpPut, "PUT"},
  {kHttpRequestMethod::kHttpDelete, "DELETE"},
  {kHttpRequestMethod::kHttpConnect, "CONNECT"},
  {kHttpRequestMethod::kHttpOptions, "OPTIONS"},
  {kHttpRequestMethod::kHttpTrace, "TRACE"},
  {kHttpRequestMethod::kHttpPatch, "PATCH"}
};

static const std::unordered_map<std::string, int> kHeaderFields = \
{
  {"Accept", kHttpHeaderField::kAccept},
  {"Content-Type", kHttpHeaderField::kContentType},
  {"User-Agent", kHttpHeaderField::kUserAgent},
  {"Content-Length", kHttpHeaderField::kContentLength},
  {"Host", kHttpHeaderField::kHost},
  {"Connection", kHttpHeaderField::kConnection}
};

static const std::unordered_map<int, std::string> kHeaderFieldStrs = \
{
  {kHttpHeaderField::kAccept, "Accept"},
  {kHttpHeaderField::kContentType, "Content-Type"},
  {kHttpHeaderField::kUserAgent, "User-Agent"},
  {kHttpHeaderField::kContentLength, "Content-Length"},
  {kHttpHeaderField::kContentEncoding, "Content-Encoding"},
  {kHttpHeaderField::kHost, "Host"},
  {kHttpHeaderField::kConnection, "Connection"}
};

} // namespace http
} // namespace ladder
