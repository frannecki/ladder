#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __unix__
#include <unistd.h>
#elif defined(_MSC_VER)
#include <Channel.h>
#endif
#include <MemoryPool.h>
#include <utils.h>

namespace ladder {

static const int kFileBufferSize = 1024;
static const int kMaxMessageLength = 1024;

void exit_fatal(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char msg[kMaxMessageLength];
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);
#ifdef _MSC_VER
  fprintf(stderr, "%s %d\n", msg, GetLastError());
#else
  perror(msg);
#endif
  exit(EXIT_FAILURE);
}

std::vector<int> FindSubstr(const std::string& str, const std::string& pat) {
  int len1 = str.size();
  int len2 = pat.size();
  std::vector<int> positions;

  MemoryWrapper<int> wrapper(1 + len2);
  int* next = wrapper.get();

  int t = next[0] = -1, i = 0, j;
  while (i < len2) {
    if (t < 0 || pat[t] == pat[i]) {
      ++i;
      int tmp = ++t;
      while (tmp >= 0 && pat[tmp] == pat[i]) {
        tmp = next[tmp];
      }
      next[i] = tmp;
    } else
      t = next[t];
  }
  i = j = 0;
  while (i + len2 <= len1 && j <= len2) {
    if (j < len2 && (j < 0 || str[i + j] == pat[j])) {
      ++j;
    } else {
      if (j == len2) {
        positions.push_back(i);
      }
      i += j - next[j];
      j = next[j];
    }
  }
  next = nullptr;

  return positions;
}

bool CheckIfFileExists(const std::string& path) {
#ifdef __unix__
  return access(path.c_str(), F_OK | R_OK) == 0;
#elif defined(_MSC_VER)
  if (INVALID_FILE_ATTRIBUTES == GetFileAttributes((LPCWSTR)(path.c_str())) && GetLastError() == ERROR_FILE_NOT_FOUND) {
    return false;
  }
  return true;
#endif
}

int GetFileSize(const std::string& path) {
  FILE* fp = fopen(path.c_str(), "r");
  if (fp == NULL) {
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  fclose(fp);
  return size;
}

std::string ReadFileAsString(const std::string& path) {
  std::string result;
  FILE* fp = fopen(path.c_str(), "rb");
  if (fp == nullptr) {
    return result;
  }

  unsigned char in_buf[kFileBufferSize] = {0};

  while (!feof(fp)) {
    int len = fread(in_buf, 1, kFileBufferSize, fp);
    result += std::string(in_buf, in_buf + len);
  }
  fclose(fp);

  return std::move(result);
}

void SslInit() {
  SSL_library_init();
  OpenSSL_add_ssl_algorithms();
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
}

SSL_CTX* CreateSslContext(bool server) {
  SSL_CTX* ctx;

  if (server)
    ctx = SSL_CTX_new(SSLv23_server_method());
  else
    ctx = SSL_CTX_new(SSLv23_client_method());

  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    EXIT("SSL_CTX_new");
  }

  return ctx;
}

void ConfigureSslContext(SSL_CTX* ctx, const char* cert_path,
                         const char* key_path) {
  SSL_CTX_set_ecdh_auto(ctx, 1);

  /* Set the key and cert */
  if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    EXIT("SSL_CTX_use_certificate_file");
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    EXIT("SSL_CTX_use_PrivateKey_file");
  }

  SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
}

#ifdef _MSC_VER
HANDLE UpdateIocpPort(HANDLE port, const Channel* channel) {
  HANDLE updated_port = CreateIoCompletionPort((HANDLE)(channel->fd()), port,
                                               (DWORD_PTR)channel, 0);
  if (updated_port == NULL) {
    EXIT("CreateIoCompletionPort");
  }
  return updated_port;
}
#endif

}  // namespace ladder
