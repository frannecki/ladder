#ifndef LADDER_GZIP_H
#define LADDER_GZIP_H

#include <string>

namespace ladder {

class GZipper {
 public:
  static std::string Deflate(const std::string& buf);
  static std::string Inflate(const std::string& buf);
  static std::string DeflateFile(const std::string& filename);
};

}  // namespace ladder

#endif
