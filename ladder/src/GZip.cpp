#include <zlib.h>

#include <GZip.h>

#include <string.h>

namespace ladder {

static const int kGZipBufferSize = 128;
static const int kGZipFileBufferSize = 128 * 1024;  // 128 kB
static const int kDeflationLevel = Z_DEFAULT_COMPRESSION;

// Reference: https://www.zlib.net/zlib_how.html

std::string GZipper::Deflate(const std::string& buf) {
  std::string result;
  if(buf.empty())  return result;
  int ret, flush;
  int bytes_finished = 0, bytes_total = buf.size();
  unsigned char out_buf[kGZipBufferSize] = {0};
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit2(&strm, kDeflationLevel, Z_DEFLATED, 16 | MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

  if (ret != Z_OK) {
    return result;
  }

  while(bytes_finished < bytes_total) {
    strm.avail_in = std::min(bytes_total - bytes_finished,
                             kGZipBufferSize);
    strm.next_in = reinterpret_cast<Bytef*>(
      const_cast<char*>(buf.c_str() + bytes_finished));
    bytes_finished += strm.avail_in;
    flush = (bytes_finished >= bytes_total) ? Z_FINISH : Z_NO_FLUSH;

    while(1) {
      strm.avail_out = kGZipBufferSize;
      strm.next_out = out_buf;
      ret = deflate(&strm, flush);  // should flush on finish
      result += std::string(reinterpret_cast<char*>(out_buf),
                            kGZipBufferSize - strm.avail_out);
      if(strm.avail_out != 0)  break;
    }
  }
  deflateEnd(&strm);

  return result;
}

std::string GZipper::Inflate(const std::string& buf) {
  std::string result;
  if(buf.empty())  return result;
  int ret, flush;
  int bytes_finished = 0, bytes_total = buf.size();
  unsigned char out_buf[kGZipBufferSize] = {0};
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit2(&strm, 16 | MAX_WBITS);

  if (ret != Z_OK) {
    return result;
  }

  while(bytes_finished < bytes_total) {
    strm.avail_in = std::min(bytes_total - bytes_finished,
                             kGZipBufferSize);
    strm.next_in = reinterpret_cast<Bytef*>(
      const_cast<char*>(buf.c_str() + bytes_finished));
    bytes_finished += strm.avail_in;
    flush = (bytes_finished >= bytes_total) ? Z_FINISH : Z_NO_FLUSH;

    while(1) {
      strm.avail_out = kGZipBufferSize;
      strm.next_out = out_buf;
      ret = inflate(&strm, flush);
      result += std::string(reinterpret_cast<char*>(out_buf),
                            kGZipBufferSize - strm.avail_out);
      if(strm.avail_out != 0)  break;
    }
  }
  inflateEnd(&strm);

  return result;
}

std::string GZipper::DeflateFile(const std::string& filename) {
  std::string result;
  FILE* fp = fopen(filename.c_str(), "rb");
  if(fp == nullptr) {
    return result;
  }

  int ret, flush = Z_NO_FLUSH;
  unsigned char in_buf[kGZipFileBufferSize] = {0};
  unsigned char out_buf[kGZipFileBufferSize] = {0};
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit2(&strm, kDeflationLevel, Z_DEFLATED, 16 | MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

  if (ret != Z_OK) {
    return result;
  }

  while(flush != Z_FINISH) {
    strm.avail_in = fread(in_buf, 1, kGZipFileBufferSize, fp);
    if (ferror(fp)) {
        deflateEnd(&strm);
        return std::string();
    }
    flush = feof(fp) ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in_buf;

    while(1) {
      strm.avail_out = kGZipFileBufferSize;
      strm.next_out = out_buf;
      ret = deflate(&strm, flush);
      result += std::string(reinterpret_cast<char*>(out_buf),
                            kGZipFileBufferSize - strm.avail_out);
      if(strm.avail_out != 0)  break;
    }
  }
  deflateEnd(&strm);

  return result;
}

} // namespace ladder
