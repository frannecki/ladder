#ifndef LADDER_UTILS_H
#define LADDER_UTILS_H

#include <stdio.h>

#include <string>
#include <vector>

#include <openssl/err.h>
#include <openssl/ssl.h>

#ifdef _MSC_VER
#include <winsock2.h>
#endif

namespace ladder {

const int kEnableOption = 1;

#define EXIT                                                        \
  fprintf(stderr, "[%s:%u %s] ", __FILE__, __LINE__, __FUNCTION__); \
  exit_fatal

void exit_fatal(const char* fmt, ...);

std::vector<int> FindSubstr(const std::string& str, const std::string& pat);

bool CheckIfFileExists(const std::string& path);

int GetFileSize(const std::string& path);

std::string ReadFileAsString(const std::string& path);

void SslInit();

SSL_CTX* CreateSslContext(bool server = true);

void ConfigureSslContext(SSL_CTX* ctx, const char* cert_path,
                         const char* key_path);

#ifdef _MSC_VER
const int kMaxIocpRecvSize = 1024;

class Channel;
HANDLE UpdateIocpPort(HANDLE port, const Channel* channel);

#endif

}  // namespace ladder

#endif
